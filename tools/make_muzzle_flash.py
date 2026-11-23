"""
マズルフラッシュ(銃口の発砲炎)のエフェクトを作るスクリプト。

Effekseer のエフェクト定義(.efkproj, XML)とテクスチャ(.png)を書き出し、
Effekseer のコマンドライン(-cui)で、ゲームが読む .efkefc に変換する。
Pythonの標準ライブラリだけで動く。(テクスチャの画像も、ここで計算して作る)

    python tools/make_muzzle_flash.py [--effekseer <Effekseer.exe のパス>]

出力先は Game/Assets/effect/MuzzleFlash/。
    MuzzleFlash.efkproj : Effekseerで開いて見た目を調整できる元ファイル。
    MuzzleFlash.efkefc  : ゲームが読む書き出し結果。
    Texture/            : エフェクトが使うテクスチャ。(.efkefc と同じ階層に置くこと。実行時に相対パスで読まれる)

エフェクトの向き: エフェクトの +Z が、銃口の向き(Vector3::Front)。単位はcm。
    原点 = 銃口。炎が +Z へ伸びる。
"""
import argparse
import math
import os
import random
import struct
import subprocess
import sys
import zlib
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = REPO_ROOT / "Game" / "Assets" / "effect" / "MuzzleFlash"
DEFAULT_EFFEKSEER = Path(r"C:\My Game\GameTemplate\tools\Effekseer160Win\Tool\Effekseer.exe")


# ---------------------------------------------------------------------------
# テクスチャ(RGBA PNG)
# ---------------------------------------------------------------------------
def write_png(path, width, height, pixels):
    """RGBAのPNGを書く。pixels は (r, g, b, a) の0-255の整数の、行ごとのリスト。"""
    raw = bytearray()
    for row in pixels:
        raw.append(0)  # フィルターなし
        for r, g, b, a in row:
            raw += bytes((r, g, b, a))

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


def make_glow(path, size=64):
    """丸いぼかし。中心が明るく、外へなめらかに消える。白色で、形はアルファで表す。"""
    pixels = []
    c = (size - 1) / 2.0
    for y in range(size):
        row = []
        for x in range(size):
            r = math.hypot(x - c, y - c) / c
            a = max(0.0, 1.0 - r)
            a = a * a * (3.0 - 2.0 * a)  # smoothstep
            row.append((255, 255, 255, int(round(a * 255))))
        pixels.append(row)
    write_png(path, size, size, pixels)


def make_spike(path, width=128, height=32):
    """横に細長い光の筋。中心線が明るく、両端へ向かって細く消える。(長軸が X)"""
    pixels = []
    cx = (width - 1) / 2.0
    cy = (height - 1) / 2.0
    for y in range(height):
        row = []
        for x in range(width):
            u = abs(x - cx) / cx  # 0(中心)-1(端)
            v = abs(y - cy) / cy
            along = (1.0 - u) ** 1.2
            # 端へいくほど細くなる
            half = max(0.02, 1.0 - u * 0.9)
            across = max(0.0, 1.0 - v / half)
            across = across * across
            a = along * across
            row.append((255, 255, 255, int(round(min(1.0, a) * 255))))
        pixels.append(row)
    write_png(path, width, height, pixels)


# ---------------------------------------------------------------------------
# エフェクト定義(.efkproj)
# ---------------------------------------------------------------------------
def rng(center, lo=None, hi=None):
    """Center/Max/Min のブロック。lo/hi を省略すると固定値。"""
    lo = center if lo is None else lo
    hi = center if hi is None else hi
    return f"<Center>{center}</Center><Max>{hi}</Max><Min>{lo}</Min>"


def axes(**kw):
    """X/Y/Zの要素。値は (center, lo, hi) か、単独の数値。"""
    out = ""
    for name in ("X", "Y", "Z"):
        if name in kw:
            v = kw[name]
            if isinstance(v, tuple):
                out += f"<{name}>{rng(*v)}</{name}>"
            else:
                out += f"<{name}>{v}</{name}>"
    return out


class Node:
    """Effekseerのノード(1種類のパーティクル)。"""

    def __init__(self, name, *, texture, blend, life, count=1, interval=0.01, delay=0,
                 billboard=0, color=(255, 255, 255, 255), fade_in=0, fade_out=0):
        self.name = name
        self.texture = texture
        self.blend = blend          # 1:通常 2:加算
        self.life = life            # (center, lo, hi) か数値
        self.count = count
        self.interval = interval
        self.delay = delay
        self.billboard = billboard  # 0:カメラ向き 2:固定 3:カメラ向きで回転できる
        self.color = color
        self.fade_in = fade_in
        self.fade_out = fade_out
        self.location = ""
        self.rotation = ""
        self.scaling = ""

    def xml(self):
        life = self.life if isinstance(self.life, tuple) else (self.life,)
        r, g, b, a = self.color
        s = "<Node>"
        s += "<CommonValues>"
        s += f"<MaxGeneration><Value>{self.count}</Value></MaxGeneration>"
        s += f"<Life>{rng(*life)}</Life>"
        s += f"<GenerationTime>{rng(self.interval)}</GenerationTime>"
        s += f"<GenerationTimeOffset>{rng(self.delay)}</GenerationTimeOffset>"
        s += "</CommonValues>"
        s += self.location + self.rotation + self.scaling
        s += "<RendererCommonValues>"
        s += f"<ColorTexture>{self.texture}</ColorTexture>"
        s += f"<AlphaBlend>{self.blend}</AlphaBlend>"
        if self.fade_in > 0:
            s += f"<FadeInType>1</FadeInType><FadeIn><Frame>{self.fade_in}</Frame></FadeIn>"
        if self.fade_out > 0:
            s += f"<FadeOutType>1</FadeOutType><FadeOut><Frame>{self.fade_out}</Frame></FadeOut>"
        s += "</RendererCommonValues>"
        s += "<DrawingValues><Sprite>"
        s += f"<Billboard>{self.billboard}</Billboard>"
        s += f"<ColorAll_Fixed><R>{r}</R><G>{g}</G><B>{b}</B><A>{a}</A></ColorAll_Fixed>"
        s += "</Sprite></DrawingValues>"
        s += f"<Name>{self.name}</Name><Children />"
        s += "</Node>"
        return s

    # 以下、位置・回転・拡大の書き方。数値の単位: 位置=cm、回転=度、拡大=倍率。速度は1フレームあたり。
    def at(self, **loc):
        self.location = f"<LocationValues><Type>0</Type><Fixed><Location>{axes(**loc)}</Location></Fixed></LocationValues>"
        return self

    def fly(self, start, velocity):
        """初速(ランダム範囲つき)で飛ぶ。start / velocity は axes() の引数(辞書)。"""
        self.location = ("<LocationValues><Type>1</Type><PVA>"
                         f"<Location>{axes(**start)}</Location><Velocity>{axes(**velocity)}</Velocity>"
                         "</PVA></LocationValues>")
        return self

    def rot(self, **r):
        self.rotation = f"<RotationValues><Type>0</Type><Fixed><Rotation>{axes(**r)}</Rotation></Fixed></RotationValues>"
        return self

    def scale(self, **sc):
        self.scaling = f"<ScalingValues><Type>0</Type><Fixed><Scale>{axes(**sc)}</Scale></Fixed></ScalingValues>"
        return self

    def scale_pva(self, size, velocity):
        """正方形のまま、大きさが1フレームごとに velocity ずつ変わる。size は (center, lo, hi)。"""
        self.scaling = ("<ScalingValues><Type>3</Type><SinglePVA>"
                        f"<Scale>{rng(*size)}</Scale><Velocity>{rng(*velocity)}</Velocity>"
                        "</SinglePVA></ScalingValues>")
        return self


def build_nodes():
    glow = "Texture/glow.png"
    spike = "Texture/spike.png"
    nodes = []

    # 中心の光の玉。すぐに縮んで消える
    nodes.append(Node("FlashGlow", texture=glow, blend=2, life=6, color=(255, 190, 90, 255), fade_out=4)
                 .at(Z=3).scale_pva((18, 18, 18), (-2.0, -2.0, -2.0)))
    # 白く光る芯
    nodes.append(Node("FlashCore", texture=glow, blend=2, life=4, color=(255, 255, 235, 255), fade_out=3)
                 .at(Z=2).scale_pva((8, 8, 8), (-1.5, -1.5, -1.5)))

    # カメラ向きの放射状の光の筋(3本。角度と長さを変える)
    for i, (angle, length) in enumerate(((0, 30), (60, 22), (120, 26))):
        nodes.append(Node(f"Ray{i}", texture=spike, blend=2, life=4, billboard=3,
                          color=(255, 210, 120, 255), fade_out=3)
                     .at(Z=3).rot(Z=angle).scale(X=length, Y=7))

    # 銃口の向きに伸びる炎(縦の面と横の面を十字に組んで、どの角度から見ても細く見えないようにする)
    # 面は元々XY平面で長軸がX。Y軸90度回すと長軸が+Zへ向く。(回転は Z→X→Y の順に適用される)
    nodes.append(Node("ConeV", texture=spike, blend=2, life=5, billboard=2,
                      color=(255, 170, 70, 255), fade_out=4)
                 .at(Z=16).rot(Y=90).scale(X=36, Y=10))
    nodes.append(Node("ConeH", texture=spike, blend=2, life=5, billboard=2,
                      color=(255, 170, 70, 255), fade_out=4)
                 .at(Z=16).rot(X=90, Y=90).scale(X=36, Y=10))

    # 火花。前方へ散らばる
    nodes.append(Node("Sparks", texture=glow, blend=2, life=(12, 8, 16), count=8, interval=0.01,
                      color=(255, 170, 60, 255), fade_out=8)
                 .fly(dict(Z=2), dict(X=(0, -1.4, 1.4), Y=(0, -1.4, 1.4), Z=(3.0, 1.8, 4.2)))
                 .scale_pva((1.6, 1.0, 2.2), (-0.05, -0.08, -0.02)))

    # 薄い煙。ゆっくり広がって上へ流れる
    nodes.append(Node("Smoke", texture=glow, blend=1, life=(30, 24, 36), count=3, interval=1.5, delay=2,
                      color=(150, 150, 150, 70), fade_in=3, fade_out=22)
                 .fly(dict(Z=4, X=(0, -1.0, 1.0)), dict(Y=(0.18, 0.1, 0.25), Z=(0.5, 0.2, 0.8)))
                 .scale_pva((5, 4, 6), (0.3, 0.2, 0.4)))

    return nodes


def build_efkproj(nodes):
    children = "".join(n.xml() for n in nodes)
    return ('<?xml version="1.0" encoding="utf-8"?>'
            "<EffekseerProject><Root><Name>Root</Name>"
            f"<Children>{children}</Children></Root>"
            "<Behavior><TargetLocation><X>11</X><Y>11</Y><Z>11</Z></TargetLocation></Behavior>"
            "<ToolVersion>1.60</ToolVersion><Version>3</Version>"
            "<StartFrame>0</StartFrame><EndFrame>60</EndFrame><IsLoop>False</IsLoop>"
            "</EffekseerProject>")


def main():
    parser = argparse.ArgumentParser(description="マズルフラッシュのエフェクトを作る")
    parser.add_argument("--effekseer", type=Path, default=DEFAULT_EFFEKSEER, help="Effekseer.exe のパス")
    args = parser.parse_args()

    random.seed(0)
    tex_dir = OUT_DIR / "Texture"
    tex_dir.mkdir(parents=True, exist_ok=True)
    make_glow(tex_dir / "glow.png")
    make_spike(tex_dir / "spike.png")

    proj = OUT_DIR / "MuzzleFlash.efkproj"
    proj.write_text(build_efkproj(build_nodes()), encoding="utf-8")
    print(f"wrote {proj}")

    efkefc = OUT_DIR / "MuzzleFlash.efkefc"
    if efkefc.exists():
        efkefc.unlink()
    if not args.effekseer.exists():
        print(f"Effekseer.exe が見つかりません: {args.effekseer}", file=sys.stderr)
        return 1
    # NOTE: 終了コードは当てにならない。(ロガーの例外が出ても書き出しは成功する) 出力ファイルの有無で確かめる。
    subprocess.run([str(args.effekseer), "-cui", "-in", str(proj), "-o", str(efkefc)],
                   cwd=args.effekseer.parent, timeout=120,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not efkefc.exists():
        print("書き出しに失敗しました", file=sys.stderr)
        return 1
    data = efkefc.read_bytes()
    pos = data.find(b"SKFE")
    version = struct.unpack("<i", data[pos + 4:pos + 8])[0] if pos >= 0 else None
    print(f"wrote {efkefc} ({len(data)} bytes, binary version {version})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
