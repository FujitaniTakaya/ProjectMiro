"""
tka(アニメーション)に、アニメーションイベントを足すスクリプト。

アニメーションイベントは、「クリップの再生時間がこの秒数を過ぎたら、この名前で通知する」という記録。
エンジン(k2EngineLow の Animation)は、再生中にその時間を過ぎると、登録されたリスナーへ名前を1回だけ通知する。
(例: ゾンビの攻撃の、腕を振り下ろす区間の始まりと終わりを知らせる)

tka のファイルは、次の並びでできている。
    ヘッダー : u32 キーの数, u32 イベントの数
    イベント : [float 発火時刻(秒), u32 名前の長さ, 名前の文字, 終端の 0 を 1 バイト] × イベントの数
    キー     : [u32 ボーン番号, float 時間, 12 個の float(4x3 行列)] × キーの数
このスクリプトは、ヘッダーのイベントの数と、イベントの部分だけを書き換える。キーの部分は 1 バイトも変えない。

同じ名前のイベントがすでにあれば、時刻だけ置き換える。違う名前のイベントは残す。
発火時刻は、0 以上でクリップの長さ(最後のキーの時間)以下だけ受け付ける。
書き出したあとで、読み直して「キーが元と同じ」「イベントが指定どおり」を確かめる。違えば書き込まずに失敗にする。

3ds Max で tka を書き出し直すと、イベントは消える。そのときは、もう一度このスクリプトを通すこと。
(tools/fix_tka_bones.py は、tka が持つイベントをそのまま引き継ぐ)

使い方:
    python tools/add_tka_event.py <tka> --event 名前=時刻 [--event 名前=時刻 ...]            # 結果を表示するだけ(書き込まない)
    python tools/add_tka_event.py <tka> --event 名前=時刻 ... --out <出力先>                 # 足した tka を書き込む

    出力先には、tka のファイル名のまま書き込む。元のファイルを置き換えるときは、--out に同じフォルダを指定する。
    (元に戻せないので、git に入っていることを確かめてから行うこと)

    例: python tools/add_tka_event.py Game/Assets/animData/character/zombie/Zombie_Attack.tka --event AttackBegin=0.55 --event AttackEnd=0.85
"""
import argparse
import struct
import sys
from pathlib import Path

from fix_tka_bones import KEY_SIZE, read_tka

# イベント名の最大の長さ(バイト)。エンジンの読み込み(TkaFile::Load)が 256 バイトの領域に読むので、終端を除いて 255
MAX_NAME_LENGTH = 255


def parse_events(data, num_event, start):
    """イベントの部分を読んで、([(時刻, 名前)], イベントの終わりの位置) を返す"""
    events = []
    pos = start
    for _ in range(num_event):
        (time,) = struct.unpack_from("<f", data, pos)
        pos += 4
        (length,) = struct.unpack_from("<I", data, pos)
        pos += 4
        name = data[pos:pos + length].decode("ascii")
        pos += length + 1
        events.append((time, name))
    return events, pos


def to_float32(value):
    """float32 に丸めた値を返す。(ファイルには float32 で入るので、比べるときも丸めた値で比べる)"""
    return struct.unpack("<f", struct.pack("<f", value))[0]


def build_events(events):
    """[(時刻, 名前)] からイベントの部分のバイト列を作る"""
    out = bytearray()
    for time, name in events:
        raw = name.encode("ascii")
        out += struct.pack("<fI", time, len(raw)) + raw + b"\0"
    return bytes(out)


def parse_event_argument(text):
    """'名前=時刻' を (名前, 時刻) にする"""
    name, separator, value = text.partition("=")
    if not separator or not name:
        raise ValueError(f"--event は 名前=時刻 の形で指定する: {text}")
    if len(name) > MAX_NAME_LENGTH or not name.isascii() or not name.isprintable():
        raise ValueError(f"イベント名は ASCII の印字できる文字で {MAX_NAME_LENGTH} 文字まで: {name}")
    try:
        time = to_float32(float(value))
    except (ValueError, OverflowError):
        raise ValueError(f"時刻が数値ではない: {text}") from None
    return name, time


def add_events(data, additions):
    """tka のバイト列にイベントを足す。(新しいバイト列, 足した後のイベント一覧, クリップの長さ) を返す"""
    num_key, num_event = struct.unpack_from("<II", data, 0)
    events, key_start = parse_events(data, num_event, 8)
    keys = data[key_start:]
    if len(keys) != num_key * KEY_SIZE:
        raise ValueError("tka の大きさがヘッダーと合わない")

    length = read_tka(data).frames[-1][0]
    for name, time in additions:
        if not (0.0 <= time <= length):
            raise ValueError(f"{name} の時刻 {time} がクリップの長さ(0〜{length:.3f}秒)の外")

    result = list(events)
    for name, time in additions:
        # 同じ名前は、時刻だけ置き換える
        for index, (_, old_name) in enumerate(result):
            if old_name == name:
                result[index] = (time, name)
                break
        else:
            result.append((time, name))

    out = struct.pack("<II", num_key, len(result)) + build_events(result) + keys
    return out, result, length


def verify(original, written, expected_events):
    """書き出した tka を読み直して、キーが元と同じで、イベントが期待どおりかを確かめる"""
    num_key, num_event = struct.unpack_from("<II", written, 0)
    events, key_start = parse_events(written, num_event, 8)
    _, original_num_event = struct.unpack_from("<II", original, 0)
    _, original_key_start = parse_events(original, original_num_event, 8)
    if written[key_start:] != original[original_key_start:]:
        raise ValueError("書き出したキーが元と違う")
    if events != expected_events:
        raise ValueError("書き出したイベントが期待と違う")
    # tka として読めること(read_tka が大きさ・ボーンの連続などを確かめる)
    read_tka(written)


def main():
    parser = argparse.ArgumentParser(description="tka にアニメーションイベントを足す")
    parser.add_argument("tka", help="tka ファイル")
    parser.add_argument("--event", action="append", required=True, metavar="名前=時刻", help="足すイベント。何個でも指定できる")
    parser.add_argument("--out", help="足した tka の出力先フォルダ。省略すると結果を表示するだけ")
    args = parser.parse_args()

    path = Path(args.tka)
    try:
        additions = [parse_event_argument(text) for text in args.event]
        original = path.read_bytes()
        _, before_count = struct.unpack_from("<II", original, 0)
        before, _ = parse_events(original, before_count, 8)
        written, events, length = add_events(original, additions)
        verify(original, written, events)
    except (ValueError, OSError, struct.error) as error:
        print(f"NG  {path.name}  {error}")
        sys.exit(1)

    print(f"{path.name}  クリップの長さ {length:.3f}秒")
    print("  今のイベント : " + (", ".join(f"{n}={t:.3f}" for t, n in before) or "なし"))
    print("  足した後     : " + ", ".join(f"{n}={t:.3f}" for t, n in events))
    if written == original:
        print("変更なし")
    if args.out:
        destination = Path(args.out) / path.name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(written)
        print(f"完了: {destination} に書き込んだ")
    else:
        print("完了。(--out が無いので書き込んでいない)")


if __name__ == "__main__":
    main()
