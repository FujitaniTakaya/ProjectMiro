"""
msdf-atlas-gen に渡す文字の一覧(charset)を作るスクリプト。

日本語の標準的な範囲の文字を、Unicodeのコードポイント(10進数)を空白で区切って1行に書き出す。

含める文字:
  - ASCII (U+0020-U+007E)
  - JIS X 0208 の記号・英数・ひらがな・カタカナ・ギリシャ・キリル・罫線・第1水準・第2水準の漢字
    (Shift_JIS の2バイトの範囲 0x81-0x84, 0x88-0x9F, 0xE0-0xEA)。∞ (U+221E) も含まれる。
  - NEC特殊文字(丸数字・ローマ数字など。cp932 の 0x87 の行)
  - 半角カナ (U+FF61-U+FF9F)
  Windowsで入力できる文字に合わせるため、shift_jis と cp932 の両方で変換して、和集合を取る。
  (例: 「～」は shift_jis だと U+301C、cp932 だと U+FF5E になる。)

含まれない文字: 絵文字、JIS第3・第4水準の漢字、IBM拡張の漢字など。必要になったら RANGES / EXTRA に足して、アトラスを作り直す。

使い方:
    python -X utf8 tools/make_charset.py <出力ファイル>
"""
import argparse
import sys
from pathlib import Path

# Shift_JIS の先頭バイトの範囲(JIS X 0208 + NEC特殊文字の行)。
LEAD_RANGES = [(0x81, 0x84), (0x87, 0x87), (0x88, 0x9F), (0xE0, 0xEA)]
# 2バイト目の範囲。
TRAIL_RANGES = [(0x40, 0x7E), (0x80, 0xFC)]
# 変換に使う文字コード。
CODECS = ["shift_jis", "cp932"]


def collect() -> set:
    codepoints = set(range(0x20, 0x7F))  # ASCII
    codepoints.update(range(0xFF61, 0xFFA0))  # 半角カナ

    for lead_first, lead_last in LEAD_RANGES:
        for lead in range(lead_first, lead_last + 1):
            for trail_first, trail_last in TRAIL_RANGES:
                for trail in range(trail_first, trail_last + 1):
                    for codec in CODECS:
                        try:
                            text = bytes([lead, trail]).decode(codec)
                        except UnicodeDecodeError:
                            continue
                        if len(text) == 1:
                            codepoints.add(ord(text))
    return codepoints


def main() -> int:
    parser = argparse.ArgumentParser(description="msdf-atlas-gen 用の charset を作る")
    parser.add_argument("output", help="出力ファイル")
    args = parser.parse_args()

    codepoints = sorted(collect())
    Path(args.output).write_text(" ".join(str(c) for c in codepoints), encoding="ascii")
    print(f"{len(codepoints)} 文字を {args.output} に書きました。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
