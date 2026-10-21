"""
tka(アニメーション)のボーン番号を tks(スケルトン)に合わせて直すスクリプト。

tka にはボーンの名前が無く、ボーン番号だけでモデルの骨を動かす。
3ds Max で tka を書き出したときのスケルトンの根元が、モデルの tks と違うと、番号が全部ずれてしまい、
アニメーションが別の骨に入って体が崩れる。(例: tks は根元の Vefects_Vexa が 0 番、tka は Pelvis が 0 番)

このスクリプトは次の順に直す。
1. tks の骨の並び・親子・バインドポーズから、各骨の「親から見た位置」を求める。
2. tka の最初のフレームの「親から見た位置」と見比べて、番号のずれを自動で判定する。
   (骨の位置関係は名前が違っても同じなので、一番よく一致するずれを選ぶ。--shift で指定もできる)
3. ずれの分だけ番号を足し、tks にあって tka に無い骨(根元・末尾の余分な骨)には変形なしのキーを足す。
4. tka の根元の骨に倍率が掛かっていたら(例: 0.394 = 1/2.54。Max の単位がインチだったとき)、割り戻す。
   位置も同じ倍率で割る。
5. 直した後のポーズの大きさを、tks のバインドポーズと比べて表示する。(極端に違うときは警告)

すでに合っている tka は変更しない。(二度掛けても同じ結果になる)
アニメーションイベントは、そのまま引き継ぐ。

対応していないもの:
- tka の方が先頭に余分な骨を持つ場合(ずれが負)。
- 骨の位置関係が違う(別のスケルトンの)tka。ずれの判定に失敗して、エラーになる。
- 座標軸(Y-up / Z-up)の違い。

使い方:
    python tools/fix_tka_bones.py --tks <tks> <tka またはフォルダ>...            # 判定して表示するだけ(書き込まない)
    python tools/fix_tka_bones.py --tks <tks> <tka またはフォルダ>... --out <出力先>  # 直した tka を書き込む

    フォルダを渡すと、その中の tka を再帰的に探す。出力先には、渡したフォルダからの相対パスのまま書き込む。
    (tka ファイルを直接渡したときは、出力先の直下にファイル名で書き込む)
    元のファイルを置き換えるときは、--out に同じフォルダを指定する。(元に戻せないので、先にコピーしておくこと)

    例: python tools/fix_tka_bones.py --tks Game/Assets/modelData/character/miro/miro.tks Game/Assets/animData/character/miro
        python tools/fix_tka_bones.py --tks Game/Assets/modelData/character/miro/miro.tks Game/Assets/animData/character/miro --out build/tka_fixed
"""
import argparse
import math
import struct
import sys
from pathlib import Path

# tka のキー 1 つの大きさ(ボーン番号 4 + 時間 4 + 4x3 行列 48)
KEY_SIZE = 56
# 変形なしの 4x3 行列
IDENTITY = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0)
# 親から見た位置が「一致」とみなす誤差(骨の長さに対する割合)
MATCH_TOLERANCE = 0.05
# ずれの判定に必要な、一致した骨の割合と数
MIN_MATCH_RATE = 0.9
MIN_MATCH_COUNT = 8
# 倍率を「掛かっている」とみなすしきい値
SCALE_EPSILON = 1e-3
# 直した後のポーズの大きさ(バインドポーズ比)がこの範囲を外れたら警告する
SIZE_RATIO_RANGE = (0.6, 1.4)


class Bone:
    """tks の骨 1 本"""

    def __init__(self, name, parent, bind):
        self.name = name
        self.parent = parent
        self.bind = bind


def read_tks(path):
    """tks を読み込んで、骨のリストを返す"""
    data = Path(path).read_bytes()
    pos = 0
    (num_bone,) = struct.unpack_from("<i", data, pos)
    pos += 4
    bones = []
    for _ in range(num_bone):
        name_len = data[pos]
        pos += 1
        name = data[pos:pos + name_len].decode("ascii", "replace")
        pos += name_len + 1
        (parent,) = struct.unpack_from("<i", data, pos)
        pos += 4
        bind = struct.unpack_from("<12f", data, pos)
        # バインドポーズの逆行列(12 float)は使わないので読み飛ばす
        pos += 96
        bones.append(Bone(name, parent, bind))
    return bones


class Tka:
    """tka を読み込んだもの。frames は [(時間, {ボーン番号: 4x3行列})]"""

    def __init__(self, num_event, events, frames):
        self.num_event = num_event
        self.events = events
        self.frames = frames


def read_tka(data):
    """tka のバイト列を読み込む"""
    num_key, num_event = struct.unpack_from("<II", data, 0)
    pos = 8
    event_start = pos
    for _ in range(num_event):
        pos += 4
        (length,) = struct.unpack_from("<I", data, pos)
        pos += 4 + length + 1
    events = data[event_start:pos]

    frames = []
    frame_index = {}
    for _ in range(num_key):
        bone, time = struct.unpack_from("<If", data, pos)
        matrix = struct.unpack_from("<12f", data, pos + 8)
        pos += KEY_SIZE
        if time not in frame_index:
            frame_index[time] = len(frames)
            frames.append((time, {}))
        frames[frame_index[time]][1][bone] = matrix
    if pos != len(data):
        raise ValueError("tka の大きさがヘッダーと合わない")

    bone_set = sorted(frames[0][1])
    for _, keys in frames:
        if sorted(keys) != bone_set:
            raise ValueError("フレームごとにキーを持つボーンが違う")
    if bone_set != list(range(len(bone_set))):
        raise ValueError("ボーン番号が 0 から連続していない")
    return Tka(num_event, events, frames)


def write_tka(tka, num_bone, frames):
    """tka のバイト列を作る。frames は [(時間, {ボーン番号: 4x3行列})]"""
    out = bytearray(struct.pack("<II", len(frames) * num_bone, tka.num_event))
    out += tka.events
    for time, keys in frames:
        for bone in range(num_bone):
            out += struct.pack("<If", bone, time)
            out += struct.pack("<12f", *keys[bone])
    return bytes(out)


def row_length(matrix, row):
    return math.sqrt(sum(v * v for v in matrix[row * 3:row * 3 + 3]))


def matrix_multiply(a, b):
    """4x3 行列(行ベクトル)の積 a * b"""
    result = []
    for r in range(4):
        for c in range(3):
            v = sum(a[r * 3 + k] * b[k * 3 + c] for k in range(3))
            if r == 3:
                v += b[9 + c]
            result.append(v)
    return tuple(result)


def local_offset(bones, index):
    """tks の骨 index の、親のバインドポーズから見た位置。(親がなければ None)"""
    bone = bones[index]
    if bone.parent < 0:
        return None
    parent = bones[bone.parent].bind
    diff = [bone.bind[9 + i] - parent[9 + i] for i in range(3)]
    # 親の回転(正規化)の逆行列は転置
    lengths = [row_length(parent, r) or 1.0 for r in range(3)]
    return [sum(diff[k] * parent[j * 3 + k] / lengths[j] for k in range(3)) for j in range(3)]


def count_matches(bones, first_keys, shift):
    """tka の骨 k が tks の骨 k+shift だとしたときに、親から見た位置が合う骨の数と、数えた骨の数"""
    matched = 0
    counted = 0
    for k in range(len(first_keys)):
        index = k + shift
        if index >= len(bones):
            break
        parent = bones[index].parent
        # 親が tka に無い骨(tka の根元)の位置は、親から見た位置にならないので数えない
        if parent < shift:
            continue
        offset = local_offset(bones, index)
        length = math.sqrt(sum(v * v for v in offset))
        if length < 1.0:
            continue
        translation = first_keys[k][9:12]
        error = math.sqrt(sum((offset[i] - translation[i]) ** 2 for i in range(3)))
        counted += 1
        if error <= MATCH_TOLERANCE * length:
            matched += 1
    return matched, counted


def detect_shift(bones, first_keys):
    """番号のずれを判定して (ずれ, 一致数, 数えた数) を返す。判定できなければ ValueError"""
    best = None
    for shift in range(0, len(bones) - len(first_keys) + 1):
        matched, counted = count_matches(bones, first_keys, shift)
        if counted == 0:
            continue
        if best is None or matched > best[1]:
            best = (shift, matched, counted)
    if best is None or best[2] < MIN_MATCH_COUNT or best[1] < MIN_MATCH_RATE * best[2]:
        raise ValueError("骨の位置関係が tks と合うずれが見つからない(別のスケルトンの tka かもしれない)")
    return best


def world_positions(bones, keys):
    """1 フレーム分のキーから、全ての骨のワールド位置を求める"""
    worlds = {}

    def world(index):
        if index not in worlds:
            parent = bones[index].parent
            local = keys[index]
            worlds[index] = matrix_multiply(local, world(parent)) if parent >= 0 else local
        return worlds[index]

    return [world(i)[9:12] for i in range(len(bones))]


def bounding_size(points):
    spans = [max(p[i] for p in points) - min(p[i] for p in points) for i in range(3)]
    return math.sqrt(sum(s * s for s in spans))


def convert(bones, tka, shift_override):
    """tks の並びに合わせた tka を作る。(変換後のバイト列, 変換したか, 報告の文字列) を返す"""
    num_bone = len(bones)
    num_tka_bone = len(tka.frames[0][1])
    first_keys = [tka.frames[0][1][k] for k in range(num_tka_bone)]

    if shift_override is None:
        shift, matched, counted = detect_shift(bones, first_keys)
    else:
        shift = shift_override
        matched, counted = count_matches(bones, first_keys, shift)
    if shift + num_tka_bone > num_bone:
        raise ValueError(f"ずれ {shift} だと tks の骨の数({num_bone})を超える")

    # 根元の骨の倍率。tks のバインドポーズの倍率に合わせる
    target_scale = row_length(bones[shift].bind, 0) or 1.0
    root_scale = row_length(first_keys[0], 0) / target_scale
    fix_scale = abs(root_scale - 1.0) > SCALE_EPSILON

    frames = []
    for time, keys in tka.frames:
        fixed = {}
        for bone in range(num_bone):
            k = bone - shift
            if 0 <= k < num_tka_bone:
                matrix = keys[k]
                if k == 0 and fix_scale:
                    scale = row_length(matrix, 0) / target_scale
                    matrix = tuple(v / scale for v in matrix)
                fixed[bone] = matrix
            else:
                fixed[bone] = IDENTITY
        frames.append((time, fixed))

    changed = shift != 0 or fix_scale or num_tka_bone != num_bone
    note = f"ずれ={shift:+d} (一致 {matched}/{counted})"
    if fix_scale:
        note += f"  根元の倍率 {root_scale:.3f}→1"

    # 直した後のポーズの大きさを、バインドポーズと比べる
    bind_size = bounding_size([b.bind[9:12] for b in bones])
    ratios = []
    for index in (0, len(frames) // 2, len(frames) - 1):
        ratios.append(bounding_size(world_positions(bones, frames[index][1])) / bind_size)
    note += f"  大きさ比 {min(ratios):.2f}〜{max(ratios):.2f}"
    warning = False
    if counted < MIN_MATCH_COUNT or matched < MIN_MATCH_RATE * counted:
        warning = True
        note += "  [警告: 骨の位置関係が合う数が少ない(--shift の指定が違うかもしれない)]"
    if min(ratios) < SIZE_RATIO_RANGE[0] or max(ratios) > SIZE_RATIO_RANGE[1]:
        warning = True
        note += "  [警告: ポーズの大きさがバインドポーズと大きく違う]"

    data = write_tka(tka, num_bone, frames) if changed else None
    return data, changed, note, warning


def collect_inputs(inputs):
    """(tka のパス, 出力先での相対パス) のリストを返す"""
    result = []
    for item in inputs:
        path = Path(item)
        if path.is_dir():
            for tka in sorted(path.rglob("*.tka")):
                result.append((tka, tka.relative_to(path)))
        elif path.is_file():
            result.append((path, Path(path.name)))
        else:
            raise FileNotFoundError(item)
    return result


def main():
    parser = argparse.ArgumentParser(description="tka のボーン番号を tks に合わせて直す")
    parser.add_argument("--tks", required=True, help="基準にするスケルトン(tks)")
    parser.add_argument("inputs", nargs="+", help="tka ファイル、またはそれを含むフォルダ(再帰)")
    parser.add_argument("--out", help="直した tka の出力先フォルダ。省略すると判定して表示するだけ")
    parser.add_argument("--shift", type=int, help="番号のずれを自動判定せずに指定する(tka の k 番が tks の k+shift 番)")
    args = parser.parse_args()

    bones = read_tks(args.tks)
    out_dir = Path(args.out) if args.out else None
    failed = 0
    warned = 0
    changed_count = 0
    for path, relative in collect_inputs(args.inputs):
        try:
            tka = read_tka(path.read_bytes())
            data, changed, note, warning = convert(bones, tka, args.shift)
        except ValueError as error:
            print(f"NG  {relative.as_posix()}  {error}")
            failed += 1
            continue
        status = "直す" if changed else "変更なし"
        print(f"{status:4s} {relative.as_posix()}  {note}")
        warned += warning
        changed_count += changed
        if out_dir is not None:
            destination = out_dir / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data if changed else path.read_bytes())

    where = f"、{out_dir} に書き込んだ" if out_dir else "。(--out が無いので書き込んでいない)"
    print(f"完了: 直す {changed_count} / 失敗 {failed} / 警告 {warned}{where}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
