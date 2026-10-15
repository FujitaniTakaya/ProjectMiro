"""
パラメーターの構造体・初期値・Load・Draw を、スキーマ(*.schema.json)とjsonから生成するスクリプト。

値は json(Assets/parameter/**/*.json)が正。スキーマは、型・コメント・ファイルのパスだけを持つ(値は持たない)。
スキーマを書いて実行すると、次の2つを行う。

- 無いファイル(スキーマの "h" / "cpp" / "json")を、雛形から作る。既にあるファイルは上書きしない。
- 既にある .h / .cpp の、マーカー `// [PARAM_GEN begin xxx]` と `// [PARAM_GEN end xxx]` の間だけを書き換える。
    members : ヘッダー。struct 内のメンバー宣言
    ctor    : .cpp。コンストラクターの定義全体(初期値は json の値)
    load    : .cpp。Load関数の `param.x = ParamLoader::ToFloat(root, "x", param.x);` の並び("load": "custom" のキーは除く)
    draw    : .cpp。Draw関数の `ImGui::Text(...)` の並び
  マーカーの外(ファイル先頭のdoc、#include、検証コードなど)は触らない。

スキーマ(例: Game/Assets/parameter/camera/PlayerCamera.schema.json。jsonの隣に置き、jsonと同じ基底名にする。
         Game/Assets/parameter 以下の *.schema.json を全部読む。Game/Source に置いたスキーマはエラーにする):
    {
        "struct": "PlayerCameraParameter",
        "namespace": "camera",
        "json": "Assets/parameter/camera/PlayerCamera.json",
        "h": "Source/Camera/PlayerCameraParameter.h",
        "cpp": "Source/Camera/PlayerCameraParameter.cpp",
        "params": [
            { "name": "targetOffsetUp", "type": "float", "doc": "注視点を、ターゲットの座標から上にずらす量" },
            { "name": "toCameraPos", "type": "vector3", "doc": "...", "load": "custom", "digits": 2 }
        ]
    }
    - "json" / "h" / "cpp" は Game/ からの相対パス。h・cpp は Source/ の中、json は Assets/parameter/ の中だけ指せる。
    - "params" の並び = メンバーの宣言順 = コンストラクターの初期化順。name = メンバー名 = jsonのキー。
    - type: float / int / uint32 / bool / vector2 / vector3 / vector4 / color / string
        vector4 は [x, y, z, w] をそのまま読む。color は [r, g, b, a] を0〜255で書き、C++ の値は255で割った値(ToVector4 の isConvert)。
    - 任意: "digits"(Draw で表示する小数の桁。既定は1、colorは2)、"load": "custom"(Load の行は生成せず、手書きにする)

使い方(リポジトリのルートで):
    python -X utf8 tools/param_gen.py            # 作成・更新する
    python -X utf8 tools/param_gen.py --check    # 書き込まずに、作成・更新される内容を表示する(あれば終了コード1)
    python -X utf8 tools/param_gen.py --root <別のルート>   # Game/ がある所を指す(検証用のコピーなど)

エラーがあったスキーマは、何も書き込まない。警告(jsonだけにあるキーなど)は表示するだけで、終了コードには影響しない。
"""
import argparse
import difflib
import json
import math
import re
import struct
import sys
from dataclasses import dataclass, field
from decimal import Decimal
from pathlib import Path, PurePosixPath

REPO_ROOT = Path(__file__).resolve().parent.parent

BOM = b"\xef\xbb\xbf"

MARKER_NOTE = "自動生成(tools/param_gen.py)。手で編集しない"
BEGIN_RE = re.compile(r"^([ \t]*)// \[PARAM_GEN begin (\w+)\]")
END_RE = re.compile(r"^[ \t]*// \[PARAM_GEN end (\w+)\]")
IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

# ファイルごとの、生成する領域の名前
H_REGIONS = {"members"}
CPP_REGIONS = {"ctor", "load", "draw"}

# 型 -> (C++の型, ヘッダーで必要なstdヘッダー, jsonに書く0の値)
TYPES = {
    "float": ("float", None, "0.0"),
    "int": ("int", None, "0"),
    "uint32": ("uint32_t", "cstdint", "0"),
    "bool": ("bool", None, "false"),
    "vector2": ("Vector2", None, "[0.0, 0.0]"),
    "vector3": ("Vector3", None, "[0.0, 0.0, 0.0]"),
    "vector4": ("Vector4", None, "[0.0, 0.0, 0.0, 0.0]"),
    "color": ("Vector4", None, "[0.0, 0.0, 0.0, 0.0]"),
    "string": ("std::string", "string", '""'),
}
VECTOR_SIZES = {"vector2": 2, "vector3": 3, "vector4": 4, "color": 4}
COMPONENT_NAMES = ["x", "y", "z", "w"]


class ParamGenError(Exception):
    """スキーマ・json・既存ファイルの問題。メッセージをそのまま表示する"""


@dataclass
class Param:
    name: str
    type: str
    doc: str
    digits: int
    custom: bool


@dataclass
class Schema:
    path: Path
    struct: str
    namespace: str
    json_path: str
    h_path: str
    cpp_path: str
    params: list

    @property
    def name(self):
        """構造体名から、末尾の Parameter を除いた名前(EnParamID の名前に使う)"""
        suffix = "Parameter"
        if self.struct.endswith(suffix) and len(self.struct) > len(suffix):
            return self.struct[: -len(suffix)]
        return self.struct


@dataclass
class Result:
    errors: list = field(default_factory=list)
    warnings: list = field(default_factory=list)
    notes: list = field(default_factory=list)
    writes: list = field(default_factory=list)  # (Path, bytes)
    created: list = field(default_factory=list)  # Path
    updated: list = field(default_factory=list)  # Path

    @property
    def changed(self):
        return bool(self.created or self.updated)


# ---------------------------------------------------------------------------
# スキーマの読み込み
# ---------------------------------------------------------------------------

def safe_rel_path(value, prefix, suffix, label):
    """Game/ からの相対パスとして安全かを調べる(Game/ の外を指せない)"""
    if not isinstance(value, str) or not value:
        raise ParamGenError(f'"{label}" は文字列で書く')
    if "\\" in value or value.startswith("/") or re.match(r"^[A-Za-z]:", value):
        raise ParamGenError(f'"{label}" は Game/ からの相対パスを / 区切りで書く: {value}')
    parts = PurePosixPath(value).parts
    if ".." in parts or "." in parts:
        raise ParamGenError(f'"{label}" に . や .. は使えない: {value}')
    if not value.startswith(prefix):
        raise ParamGenError(f'"{label}" は {prefix} の中だけ指せる: {value}')
    if not value.endswith(suffix):
        raise ParamGenError(f'"{label}" の拡張子は {suffix}: {value}')
    return value


def parse_schema(path):
    try:
        data = json.loads(path.read_bytes().decode("utf-8-sig"))
    except (UnicodeDecodeError, json.JSONDecodeError) as e:
        raise ParamGenError(f"スキーマを読めない: {e}")
    if not isinstance(data, dict):
        raise ParamGenError("スキーマのルートは { } にする")

    for key in ("struct", "namespace", "json", "h", "cpp", "params"):
        if key not in data:
            raise ParamGenError(f'スキーマに "{key}" が無い')

    struct = data["struct"]
    namespace = data["namespace"]
    if not isinstance(struct, str) or not IDENT_RE.match(struct):
        raise ParamGenError(f'"struct" は C++ の識別子で書く: {struct!r}')
    if not isinstance(namespace, str) or not IDENT_RE.match(namespace):
        raise ParamGenError(f'"namespace" は C++ の識別子で書く: {namespace!r}')
    json_path = safe_rel_path(data["json"], "Assets/parameter/", ".json", "json")
    h_path = safe_rel_path(data["h"], "Source/", ".h", "h")
    cpp_path = safe_rel_path(data["cpp"], "Source/", ".cpp", "cpp")

    if not isinstance(data["params"], list):
        raise ParamGenError('"params" は配列で書く')
    params = []
    seen = set()
    for i, entry in enumerate(data["params"]):
        where = f'params[{i}]'
        if not isinstance(entry, dict):
            raise ParamGenError(f"{where} は {{ }} で書く")
        name = entry.get("name")
        if not isinstance(name, str) or not IDENT_RE.match(name):
            raise ParamGenError(f'{where}: "name" は C++ の識別子で書く: {name!r}')
        where = f'"{name}"'
        if name in seen:
            raise ParamGenError(f"{where}: name が重複している")
        seen.add(name)
        ptype = entry.get("type")
        if ptype not in TYPES:
            raise ParamGenError(f'{where}: "type" は {" / ".join(TYPES)} のどれか: {ptype!r}')
        doc = entry.get("doc")
        if not isinstance(doc, str) or not doc.strip() or "\n" in doc or "\r" in doc or "*/" in doc:
            raise ParamGenError(f'{where}: "doc" は、1行の説明(*/ を含まない)を書く')
        digits = entry.get("digits", 2 if ptype == "color" else 1)
        if isinstance(digits, bool) or not isinstance(digits, int) or not 0 <= digits <= 9:
            raise ParamGenError(f'{where}: "digits" は0〜9の整数')
        load = entry.get("load", "auto")
        if load not in ("auto", "custom"):
            raise ParamGenError(f'{where}: "load" は "auto" か "custom": {load!r}')
        unknown = set(entry) - {"name", "type", "doc", "digits", "load"}
        if unknown:
            raise ParamGenError(f"{where}: 知らない項目 {sorted(unknown)}")
        params.append(Param(name, ptype, doc, digits, load == "custom"))

    return Schema(path, struct, namespace, json_path, h_path, cpp_path, params)


# ---------------------------------------------------------------------------
# 値の変換(jsonの値 -> C++のリテラル)
# ---------------------------------------------------------------------------

def f32(x):
    """doubleをfloat32に丸める"""
    try:
        return struct.unpack("<f", struct.pack("<f", x))[0]
    except OverflowError:
        raise ParamGenError(f"floatに収まらない値: {x}")


def float_literal(x):
    """float32に丸めた値が元に戻る、いちばん短い桁数のリテラル(例 78.0f, -0.2f, 10000.0f)"""
    if not math.isfinite(float(x)):
        raise ParamGenError(f"有限でない値: {x}")
    v = f32(float(x))
    text = ""
    for n in range(1, 10):
        text = format(v, f".{n}g")
        try:
            if f32(float(text)) == v:
                break
        except ParamGenError:
            continue  # 桁を切り詰めた値が floatの範囲を超えた(FLT_MAX付近)。桁数を増やす
    plain = format(Decimal(text), "f")
    if "." not in plain:
        plain += ".0"
    return plain + "f"


def string_literal(s):
    out = ['"']
    for ch in s:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\r":
            out.append("\\r")
        elif ch == "\t":
            out.append("\\t")
        elif ord(ch) < 0x20 or ord(ch) == 0x7F:
            raise ParamGenError(f"文字列に使えない制御文字がある: U+{ord(ch):04X}")
        else:
            try:
                ch.encode("utf-8")
            except UnicodeEncodeError:
                raise ParamGenError("文字列にUTF-8にできない文字がある")
            out.append(ch)
    out.append('"')
    return "".join(out)


def is_number(v):
    return isinstance(v, (int, float)) and not isinstance(v, bool)


def initializer_args(param, value, warnings):
    """コンストラクターの初期化子 `name(...)` の中身。ParamLoader の読み込み方と同じ条件で調べる"""
    t = param.type
    if t == "float":
        if not is_number(value):
            raise ParamGenError("数値で書く")
        return float_literal(value)
    if t == "int":
        # ToInt は is_number_integer()。1.0 のような小数表記は無効
        if isinstance(value, bool) or not isinstance(value, int):
            raise ParamGenError("整数で書く(1.0 のような小数は不可)")
        if not -(2**31) <= value <= 2**31 - 1:
            raise ParamGenError("intに収まらない値")
        return str(value)
    if t == "uint32":
        # ToUInt32 は is_number_unsigned()。負の数・小数は無効
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise ParamGenError("0以上の整数で書く")
        if value > 2**32 - 1:
            raise ParamGenError("uint32に収まらない値")
        return str(value)
    if t == "bool":
        if not isinstance(value, bool):
            raise ParamGenError("true か false で書く")
        return "true" if value else "false"
    if t == "string":
        if not isinstance(value, str):
            raise ParamGenError("文字列で書く")
        return string_literal(value)
    size = VECTOR_SIZES[t]
    if not isinstance(value, list) or len(value) != size or not all(is_number(c) for c in value):
        raise ParamGenError(f"数値{size}個の配列で書く")
    if t == "color":
        out = []
        for c in value:
            q = f32(f32(float(c)) / 255.0)  # ToVector4: component / 255.0f(floatの除算)
            clamped = min(max(q, 0.0), 1.0)  # std::clamp(…, 0.0f, 1.0f)
            if clamped != q:
                warnings.append(f'"{param.name}": {c} は0〜255の外。ParamLoader と同じく0〜1に丸める')
            out.append(float_literal(clamped))
        return ", ".join(out)
    return ", ".join(float_literal(c) for c in value)


def zero_json_value(ptype):
    return TYPES[ptype][2]


def guess_type(value):
    """jsonだけにあるキーの、スキーマに足す型の推測(警告に表示するだけ)"""
    if isinstance(value, bool):
        return "bool"
    if isinstance(value, int):
        return "int"
    if isinstance(value, float):
        return "float"
    if isinstance(value, str):
        return "string"
    if isinstance(value, list) and len(value) in (2, 3, 4) and all(is_number(c) for c in value):
        return f"vector{len(value)}"
    return None


# ---------------------------------------------------------------------------
# 生成する行(マーカーの間)。先頭の字下げは、マーカー行に合わせて呼び出し側が付ける
# ---------------------------------------------------------------------------

def gen_members(schema, _literals):
    out = []
    for p in schema.params:
        out.append(f"/** {p.doc} */")
        out.append(f"{TYPES[p.type][0]} {p.name};")
    return out


def gen_ctor(schema, literals):
    s = schema.struct
    out = [f"{s}::{s}()"]
    for k, p in enumerate(schema.params):
        out.append(("    : " if k == 0 else "    , ") + f"{p.name}({literals[p.name]})")
    out += ["{", "}"]
    return out


def gen_load(schema, _literals):
    out = []
    for p in schema.params:
        if p.custom:
            continue
        if p.type == "color":
            call = f'ParamLoader::ToVector4(root, "{p.name}", true, param.{p.name})'
        elif p.type == "vector4":
            call = f'ParamLoader::ToVector4(root, "{p.name}", false, param.{p.name})'
        else:
            func = {"float": "ToFloat", "int": "ToInt", "uint32": "ToUInt32", "bool": "ToBool", "string": "ToString", "vector2": "ToVector2", "vector3": "ToVector3"}[p.type]
            call = f'ParamLoader::{func}(root, "{p.name}", param.{p.name})'
        out.append(f"param.{p.name} = {call};")
    return out


def gen_draw(schema, _literals):
    out = []
    for p in schema.params:
        n, d = p.name, p.digits
        if p.type == "float":
            out.append(f'ImGui::Text("{n}: %.{d}f", param.{n});')
        elif p.type == "int":
            out.append(f'ImGui::Text("{n}: %d", param.{n});')
        elif p.type == "uint32":
            out.append(f'ImGui::Text("{n}: %u", param.{n});')
        elif p.type == "bool":
            out.append(f'ImGui::Text("{n}: %s", param.{n} ? "true" : "false");')
        elif p.type == "string":
            out.append(f'ImGui::Text("{n}: %s", param.{n}.c_str());')
        else:
            size = VECTOR_SIZES[p.type]
            fmt = ", ".join([f"%.{d}f"] * size)
            args = ", ".join(f"param.{n}.{COMPONENT_NAMES[i]}" for i in range(size))
            out.append(f'ImGui::Text("{n}: ({fmt})", {args});')
    return out


GENERATORS = {"members": gen_members, "ctor": gen_ctor, "load": gen_load, "draw": gen_draw}


# ---------------------------------------------------------------------------
# 雛形
# ---------------------------------------------------------------------------

def std_headers_needed(schema):
    return sorted({TYPES[p.type][1] for p in schema.params if TYPES[p.type][1]})


def header_template(schema):
    name = schema.name
    s = schema.struct
    lines = [
        "/**",
        f" * @file {PurePosixPath(schema.h_path).name}",
        f" * @brief {name}のパラメーター",
        f" * @details ParamHolderに EnParamID::{name} で登録する。jsonは {schema.json_path}。",
        f" *          例: const {s}& param = ParamHolder::Get().GetParameter<{s}>(EnParamID::{name});",
        " */",
        "#pragma once",
    ]
    std = std_headers_needed(schema)
    for h in std:
        lines.append(f"#include <{h}>")
    if std:
        lines.append("")
    lines += [
        '#include "Source/Core/Json/JsonView.h"',
        "",
        "",
        "namespace app",
        "{",
        f"    namespace {schema.namespace}",
        "    {",
        "        /**",
        f"         * @brief {name}のパラメーター",
        "         */",
        f"        struct {s}",
        "        {",
        f"            {s}();",
        "",
        "",
        f"            // [PARAM_GEN begin members] {MARKER_NOTE}",
        "            // [PARAM_GEN end members]",
        "        };",
        "",
        "",
        "",
        "",
        "        /*************************************************************/",
        "",
        "",
        "        /**",
        f"         * @brief jsonから{name}のパラメーターを読み込む",
        "         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。",
        "         * @param root jsonのルート",
        "         * @param[out] param 読み込み先。デフォルト構築された状態で渡される",
        "         */",
        f"        void Load{s}(const JsonView& root, {s}& param);",
        "",
        "        /**",
        f"         * @brief {name}のパラメーターを、デバッグ画面(ParamDebugUI)に表示する",
        "         * @param param 表示するパラメーター",
        "         */",
        f"        void Draw{s}(const {s}& param);",
        f"    }} // namespace {schema.namespace}",
        "} // namespace app",
        "",
    ]
    return "\n".join(lines)


def cpp_template(schema):
    s = schema.struct
    h_dir = PurePosixPath(schema.h_path).parent
    cpp_dir = PurePosixPath(schema.cpp_path).parent
    own_header = PurePosixPath(schema.h_path).name if h_dir == cpp_dir else schema.h_path
    lines = [
        "/**",
        f" * @file {PurePosixPath(schema.cpp_path).name}",
        f" * @brief {schema.name}のパラメーター",
        " */",
        '#include "stdafx.h"',
        "",
        f'#include "{own_header}"',
        "",
        '#include "Source/Core/Json/ParamLoader.h"',
        "",
        "",
        "namespace app",
        "{",
        f"    namespace {schema.namespace}",
        "    {",
        f"        // [PARAM_GEN begin ctor] {MARKER_NOTE}",
        "        // [PARAM_GEN end ctor]",
        "",
        "",
        "",
        "",
        "        /*************************************************************/",
        "",
        "",
        f"        void Load{s}(const JsonView& root, {s}& param)",
        "        {",
        f"            // [PARAM_GEN begin load] {MARKER_NOTE}",
        "            // [PARAM_GEN end load]",
        "        }",
        "",
        "",
        f"        void Draw{s}([[maybe_unused]] const {s}& param)",
        "        {",
        "#ifdef BALLOON_IMGUI_ENABLED",
        f"            // [PARAM_GEN begin draw] {MARKER_NOTE}",
        "            // [PARAM_GEN end draw]",
        "#endif // BALLOON_IMGUI_ENABLED",
        "        }",
        f"    }} // namespace {schema.namespace}",
        "} // namespace app",
        "",
    ]
    return "\n".join(lines)


def zero_json_text(schema):
    lines = ["{"]
    for k, p in enumerate(schema.params):
        comma = "," if k < len(schema.params) - 1 else ""
        lines.append(f'    "{p.name}": {zero_json_value(p.type)}{comma}')
    lines.append("}")
    return "\r\n".join(lines) + "\r\n"


# ---------------------------------------------------------------------------
# ファイルの読み書き・マーカーの書き換え
# ---------------------------------------------------------------------------

def decode_source(data, label):
    bom = data.startswith(BOM)
    body = data[len(BOM):] if bom else data
    try:
        text = body.decode("utf-8")
    except UnicodeDecodeError:
        raise ParamGenError(f"{label}: UTF-8ではない")
    eol = "\r\n" if "\r\n" in text else "\n"
    return bom, eol, text


def encode_source(bom, eol, text, normalize):
    """バイト列にする。normalize(雛形から作ったとき)だけ、全ての行末を eol に揃える。既存のファイルは、変わらない行の行末をそのまま保つ"""
    if normalize and eol == "\r\n":
        text = text.replace("\r\n", "\n").replace("\n", "\r\n")
    return (BOM if bom else b"") + text.encode("utf-8")


def find_regions(lines, allowed, label):
    """マーカーの位置を調べる。 {名前: (beginの行, endの行, 字下げ)}"""
    found = {}
    i = 0
    while i < len(lines):
        raw = lines[i].rstrip("\r")
        m = BEGIN_RE.match(raw)
        if m:
            indent, rid = m.group(1), m.group(2)
            if rid not in allowed:
                raise ParamGenError(f"{label}:{i + 1}: このファイルに無い領域 '{rid}' (使えるのは {sorted(allowed)})")
            if rid in found:
                raise ParamGenError(f"{label}:{i + 1}: 領域 '{rid}' が2回ある")
            j = i + 1
            while j < len(lines):
                raw2 = lines[j].rstrip("\r")
                if BEGIN_RE.match(raw2):
                    raise ParamGenError(f"{label}:{j + 1}: '{rid}' の end より前に begin がある")
                m2 = END_RE.match(raw2)
                if m2:
                    if m2.group(1) != rid:
                        raise ParamGenError(f"{label}:{j + 1}: end の名前が begin('{rid}')と違う: '{m2.group(1)}'")
                    break
                j += 1
            else:
                raise ParamGenError(f"{label}:{i + 1}: '{rid}' の end が無い")
            found[rid] = (i, j, indent)
            i = j + 1
        else:
            if END_RE.match(raw):
                raise ParamGenError(f"{label}:{i + 1}: begin の無い end")
            i += 1
    missing = sorted(allowed - set(found))
    if missing:
        raise ParamGenError(f"{label}: マーカーが無い領域 {missing}。`// [PARAM_GEN begin xxx]` と `// [PARAM_GEN end xxx]` を書く")
    return found


def fill_regions(text, eol, schema, literals, allowed, label):
    """マーカーの間を生成した行に置き換える。 (新しいtext, {領域: (古い行, 新しい行)}の変わったもの)"""
    lines = text.split("\n")
    found = find_regions(lines, allowed, label)
    suffix = "\r" if eol == "\r\n" else ""
    changes = {}
    for rid, (b, e, indent) in sorted(found.items(), key=lambda kv: -kv[1][0]):
        generated = [(indent + ln if ln else "") for ln in GENERATORS[rid](schema, literals)]
        old = [ln.rstrip("\r") for ln in lines[b + 1:e]]
        if old != generated:
            changes[rid] = (old, generated)
            lines[b + 1:e] = [ln + suffix for ln in generated]
    return "\n".join(lines), changes


def describe_changes(changes):
    out = []
    for rid in sorted(changes):
        old, new = changes[rid]
        out.append(f"    [{rid}]")
        for ln in difflib.unified_diff(old, new, lineterm="", n=0):
            if ln.startswith(("---", "+++", "@@")):
                continue
            out.append(f"      {ln}")
    return out


# ---------------------------------------------------------------------------
# スキーマ1つぶんの処理
# ---------------------------------------------------------------------------

def process_schema(game, schema, param_list_text):
    res = Result()
    rel = lambda p: p.relative_to(game.parent).as_posix()
    json_file = game / schema.json_path
    h_file = game / schema.h_path
    cpp_file = game / schema.cpp_path

    # --- json
    json_created_text = None
    if json_file.exists():
        try:
            values = json.loads(json_file.read_bytes().decode("utf-8-sig"))
        except (UnicodeDecodeError, json.JSONDecodeError) as e:
            res.errors.append(f"{rel(json_file)}: jsonを読めない: {e}")
            return res
    else:
        json_created_text = zero_json_text(schema)
        values = json.loads(json_created_text)
        res.created.append(json_file)
        res.notes.append(f"  作成: {rel(json_file)} (値は0。あとで値を入れる)")
    if not isinstance(values, dict):
        res.errors.append(f"{rel(json_file)}: ルートが {{ }} ではない")
        return res

    # --- jsonの値 -> 初期値のリテラル
    literals = {}
    for p in schema.params:
        if p.name not in values:
            res.errors.append(f'{rel(json_file)}: キー "{p.name}" が無い。値を足す')
            continue
        try:
            literals[p.name] = initializer_args(p, values[p.name], res.warnings)
        except ParamGenError as e:
            res.errors.append(f'{rel(json_file)}: "{p.name}" ({p.type}): {e}')
    names = {p.name for p in schema.params}
    for key, value in values.items():
        if key not in names:
            guessed = guess_type(value)
            hint = f'足すなら: {{ "name": "{key}", "type": "{guessed}", "doc": "" }}' if guessed else "型を推測できない"
            res.warnings.append(f'{rel(json_file)}: キー "{key}" がスキーマに無い。{hint}')
    if res.errors:
        return res

    # --- .h / .cpp
    for kind, path, allowed, template in (
        ("h", h_file, H_REGIONS, header_template),
        ("cpp", cpp_file, CPP_REGIONS, cpp_template),
    ):
        label = rel(path)
        try:
            if path.exists():
                bom, eol, text = decode_source(path.read_bytes(), label)
                is_new = False
            else:
                bom, eol, text = True, "\r\n", template(schema)
                is_new = True
            new_text, changes = fill_regions(text, eol, schema, literals, allowed, label)
            if kind == "h" and not is_new:
                for std in std_headers_needed(schema):
                    if not re.search(rf"^\s*#include\s*<{std}>", text, re.M):
                        raise ParamGenError(f"{label}: <{std}> をincludeしていない。マーカーの外なので、手で足す")
        except ParamGenError as e:
            res.errors.append(str(e))
            continue
        data = encode_source(bom, eol, new_text, is_new)
        if is_new:
            res.created.append(path)
            res.notes.append(f"  作成: {label}")
            res.writes.append((path, data))
        elif changes:
            res.updated.append(path)
            res.notes.append(f"  更新: {label} ({', '.join(sorted(changes))})")
            res.notes.extend(describe_changes(changes))
            res.writes.append((path, data))

    if res.errors:
        return res
    if json_created_text is not None:
        res.writes.append((json_file, json_created_text.encode("utf-8")))

    # --- 新しく作ったときと、ParamList.cpp に未登録のときの案内
    reg_pattern = rf"Register<\s*{re.escape(schema.namespace)}::{re.escape(schema.struct)}\s*>"
    registered = param_list_text is not None and re.search(reg_pattern, param_list_text) is not None
    if res.created or (param_list_text is not None and not registered):
        register_line = (
            f'holder.Register<{schema.namespace}::{schema.struct}>(EnParamID::{schema.name}, "{schema.json_path}", '
            f"&{schema.namespace}::Load{schema.struct}, &{schema.namespace}::Draw{schema.struct});"
        )
        todo = [
            "手でやること:" if res.created else f"{schema.struct} が Source/ParamList.cpp に登録されていない。足す:",
        ]
        if res.created:
            todo.append("  1. python -X utf8 tools/sync_vcxproj.py Game   (新しい.h/.cppを登録)")
            todo.append(f"  2. Source/Core/Param/ParamList.h の EnParamID に {schema.name} を足す")
            todo.append(f'  3. Source/ParamList.cpp に #include "{schema.h_path}" と、次の1行を足す')
        else:
            todo.append(f'  #include "{schema.h_path}" と、次の1行')
        todo.append(f"     {register_line}")
        if res.created:
            todo.append("  4. jsonに値を入れて、もう一度 param_gen.py を実行する")
        (res.notes if res.created else res.warnings).extend(todo)
    return res


# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true", help="書き込まずに、作成・更新される内容を表示する(あれば終了コード1)")
    ap.add_argument("--root", help="Game/ があるフォルダ。省略するとこのスクリプトの親の親")
    args = ap.parse_args()
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

    root = Path(args.root).resolve() if args.root else REPO_ROOT
    game = root / "Game"
    if not (game / "Source").is_dir():
        print(f"{game / 'Source'} が見つからない。--root を指定する", file=sys.stderr)
        return 2

    param_list = game / "Source" / "ParamList.cpp"
    param_list_text = param_list.read_bytes().decode("utf-8-sig") if param_list.exists() else None

    schema_root = game / "Assets" / "parameter"
    schema_files = sorted(schema_root.rglob("*.schema.json")) if schema_root.is_dir() else []
    # Source/ に置いたスキーマは読まない。黙って無視すると気づけないので、エラーにする
    stray_files = sorted((game / "Source").rglob("*.schema.json"))
    if not schema_files and not stray_files:
        print("スキーマ(Game/Assets/parameter/**/*.schema.json)が無い")
        return 0

    # スキーマを読み、同じファイルを2つのスキーマが指していないかを調べる
    schemas = []
    results = {}
    owners = {}
    for sf in stray_files:
        r = Result()
        r.errors.append("スキーマは Game/Assets/parameter/ の中、jsonの隣に置く(ここのスキーマは読まない)")
        results[sf] = r
    schema_files = schema_files + stray_files
    for sf in schema_files:
        if sf in results:
            continue
        rel_schema = sf.relative_to(root).as_posix()
        try:
            schema = parse_schema(sf)
        except ParamGenError as e:
            r = Result()
            r.errors.append(f"{rel_schema}: {e}")
            results[sf] = r
            continue
        for p in (schema.json_path, schema.h_path, schema.cpp_path):
            if p in owners:
                r = Result()
                r.errors.append(f"{rel_schema}: {p} を {owners[p]} も指している")
                results[sf] = r
                break
            owners[p] = rel_schema
        else:
            schemas.append(schema)

    for schema in schemas:
        results[schema.path] = process_schema(game, schema, param_list_text)

    total = {"created": 0, "updated": 0, "same": 0, "error": 0}
    for sf in schema_files:
        r = results[sf]
        print(f"{sf.relative_to(root).as_posix()}:")
        for line in r.notes:
            print(line)
        for line in r.warnings:
            print(f"  警告: {line}")
        for line in r.errors:
            print(f"  エラー: {line}")
        if r.errors:
            total["error"] += 1
            print("  → エラーがあるので、何も書き込まない")
            continue
        if not args.check:
            for path, data in r.writes:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
        if r.created:
            total["created"] += 1
        elif r.updated:
            total["updated"] += 1
        else:
            total["same"] += 1
            print("  変更なし")

    verb = "(--check: 書き込んでいない)" if args.check else ""
    print(f"\n{len(schema_files)}個のスキーマ: 作成あり{total['created']}、更新あり{total['updated']}、変更なし{total['same']}、エラー{total['error']} {verb}".rstrip())
    if total["error"]:
        return 1
    if args.check and (total["created"] or total["updated"]):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
