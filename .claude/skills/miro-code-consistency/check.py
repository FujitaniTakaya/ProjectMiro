#!/usr/bin/env python3
"""Miro Source の一貫性チェック。contradictions.md の「検出」列のIDに対応する。

使い方(リポジトリのルートで):
    python -X utf8 .claude/skills/miro-code-consistency/check.py
    python -X utf8 .claude/skills/miro-code-consistency/check.py --only B1,C2,F1
    python -X utf8 .claude/skills/miro-code-consistency/check.py --root <別のルート>   (HEADを展開したものなどを調べる)

違反があれば `ID  パス:行  メッセージ` を出して、終了コード1で終わる。何も読み書きせず、ファイルは変更しない。
"""
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict

# 対象外: テンプレート由来のファイル(Game.*, main.cpp, system/, stdafx.*)
EXTRA_TARGETS = ["Game/Application.h", "Game/Application.cpp", "Game/IObject.h"]

# Doxygenのタグ順(contradictions.md D3)
DOC_RANK = {"brief": 0, "details": 1, "note": 2, "tparam": 3, "param": 4, "return": 5}

# F6: 使っているstdの名前 → 必要なヘッダー
STD_TOKENS = {
    "memory": r"std::(unique_ptr|shared_ptr|make_unique|make_shared|weak_ptr)\b",
    "utility": r"std::(move|forward|pair|swap|exchange)\b",
    "vector": r"std::vector\b",
    "cstdint": r"\b(u?int(8|16|32|64)_t)\b",
    "algorithm": r"std::(clamp|min|max|sort|remove_if|find|find_if|reverse|fill|copy|any_of|all_of|count_if|lower_bound)\b|\(std::(min|max)\)",
    "cmath": r"std::(abs|fmod|sqrt|pow|floor|ceil|sin|cos|atan2|fabs|round)\b",
    "iterator": r"std::(size|begin|end|distance)\b",
    "functional": r"std::function\b",
    "map": r"std::map\b",
    "unordered_map": r"std::unordered_map\b",
    "string": r"std::(string|to_string|wstring)\b",
    "string_view": r"std::string_view\b",
    "array": r"std::array\b",
    "chrono": r"std::chrono\b",
    "limits": r"std::numeric_limits\b",
}


def find_root(start):
    d = os.path.abspath(start)
    while True:
        if os.path.isdir(os.path.join(d, "Game", "Source")):
            return d
        parent = os.path.dirname(d)
        if parent == d:
            return None
        d = parent


def target_files(root):
    files = []
    for dirpath, _, names in os.walk(os.path.join(root, "Game", "Source")):
        for n in names:
            if n.endswith((".h", ".cpp")):
                files.append(os.path.join(dirpath, n))
    for p in EXTRA_TARGETS:
        full = os.path.join(root, p)
        if os.path.exists(full):
            files.append(full)
    return sorted(files)


def strip_code(text):
    """コメントと文字列リテラルを消す(行数は保たない。トークン検出用)"""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r'"(?:[^"\\\n]|\\.)*"', '""', text)
    return text


def blank_comments_keep_lines(text):
    """コメントを空白に置き換える。行番号を保つ"""

    def blank(m):
        return re.sub(r"[^\n]", " ", m.group(0))

    text = re.sub(r"/\*.*?\*/", blank, text, flags=re.S)
    text = re.sub(r"//[^\n]*", blank, text)
    return text


def find_clang_format():
    hits = sorted(glob.glob(r"C:\Program Files\Microsoft Visual Studio\*\VC\Tools\Llvm\x64\bin\clang-format.exe"))
    if hits:
        return hits[-1]
    return shutil.which("clang-format")


class Report:
    def __init__(self, only):
        self.only = only
        self.items = []

    def wants(self, check_id):
        return self.only is None or check_id in self.only

    def add(self, check_id, path, line, message):
        self.items.append((check_id, path, line, message))


def check_files(root, files, rep, texts):
    rel = lambda p: os.path.relpath(p, root).replace(os.sep, "/")
    for p in files:
        raw = open(p, "rb").read()
        r = rel(p)
        # F2/F3/F4: 文字コード・改行・インデント
        try:
            text = raw.decode("utf-8-sig")
        except UnicodeDecodeError:
            rep.add("F2", r, 1, "UTF-8ではない(Shift-JISなど)。UTF-8(BOM付き)にする")
            text = raw.decode("cp932", errors="replace")
        else:
            if not raw.startswith(b"\xef\xbb\xbf"):
                rep.add("F2", r, 1, "BOMが無い。MSVCがcp932として読んで、C4819になる")
        crlf = raw.count(b"\r\n")
        lf = raw.count(b"\n") - crlf
        if lf:
            rep.add("F3", r, 1, f"LFの改行が{lf}行ある。CRLFにそろえる")
        if raw and not raw.endswith(b"\r\n"):
            rep.add("F3", r, 1, "末尾の改行が無い")
        text = text.replace("\r\n", "\n")
        texts[p] = text
        for i, line in enumerate(text.split("\n"), 1):
            if line.startswith("\t"):
                rep.add("F4", r, i, "行頭がタブ。4スペースにする")
                break  # 1ファイル1件

    for p in files:
        text = texts[p]
        r = rel(p)
        lines = text.split("\n")
        code_lines = blank_comments_keep_lines(text).split("\n")

        for i, line in enumerate(code_lines, 1):
            # B1: _DEBUG の使用
            if re.search(r"#\s*if(def)?\s+_DEBUG\b|defined\s*\(\s*_DEBUG\s*\)|#\s*ifndef\s+_DEBUG\b", line):
                rep.add("B1", r, i, "_DEBUGは使わない。開発用はK2_DEBUG、ImGuiはBALLOON_IMGUI_ENABLED")
            # F8: clang-formatがマクロとpublic:を結合した形
            if re.search(r"\b(public|private|protected)\s+:\s+\S", line):
                rep.add("F8", r, i, "アクセス指定子が次の宣言と結合している。セミコロン無しのマクロをStatementMacrosに登録する")
            # F9: 全角の括弧、～
            if re.search("[（）]", lines[i - 1]):
                rep.add("F9", r, i, "全角の括弧。半角にする")
            if "～" in lines[i - 1]:
                rep.add("F9", r, i, "範囲記号は〜(U+301C)にそろえる")
            # C10: タイポ
            if "Requesut" in lines[i - 1]:
                rep.add("C10", r, i, "RequesutScene → RequestScene")
            # F10: 波括弧なしの if/for/while の1行書き
            if re.match(r"\s*(if|else\s+if|for|while)\s*\(.*\)\s+(return|continue|break|[A-Za-z_][\w:.\->]*)\b.*;\s*$", line) and not line.rstrip().endswith("{"):
                rep.add("F10", r, i, "波括弧が無いif/for/while")
            if re.match(r"\s*(if|else\s+if|for|while)\s*\(.*\)\s*$", line) and i < len(code_lines):
                nxt = next((l for l in code_lines[i:] if l.strip()), "")
                if nxt.strip() and not nxt.strip().startswith("{"):
                    rep.add("F10", r, i, "波括弧が無いif/for/while(次の行が{で始まらない)")

        # C2: ヘッダーのメンバーのデフォルト初期化子(`型 m_x = ...;`)
        if p.endswith(".h"):
            for i, line in enumerate(code_lines, 1):
                if re.match(r"\s{8,}(?:static\s+|const\s+|constexpr\s+|inline\s+)*[A-Za-z_][\w:<>,*&]*[\s*&]+m_\w+\s*(=[^=].*|\{.*\})?;\s*$", line) and re.search(r"m_\w+\s*(=[^=]|\{)", line):
                    rep.add("C2", r, i, "メンバーのデフォルト初期化子。コンストラクタの初期化リストで初期化する")

        # F5: 同じモジュールなのに "Source/<Module>/X.h" のフルパス
        d = os.path.dirname(r)  # Game/Source/<Module>
        for i, line in enumerate(lines, 1):
            m = re.match(r'#include "(Game/)?(Source/[^"]+)"', line)
            if m:
                inc = m.group(2)
                if "Game/" + os.path.dirname(inc) == d:
                    rep.add("F5", r, i, f'同じディレクトリのヘッダーは素の名前で書く: "{os.path.basename(inc)}"')

        # F7: 最上位の namespace app の閉じコメント
        nonblank = [l for l in lines if l.strip()]
        if nonblank and any(re.match(r"\s*namespace\s+app\b", l) for l in nonblank):
            if nonblank[-1].strip() != "} // namespace app":
                rep.add("F7", r, len(lines), "最後の閉じが `} // namespace app` ではない")

        # D3/D6: Doxygenのタグ順と /** での開始
        i = 0
        while i < len(lines):
            if lines[i].strip() == "/**":
                j = i
                while j < len(lines) and lines[j].strip() != "*/":
                    j += 1
                tags = []
                for k in range(i + 1, min(j, len(lines))):
                    m = re.match(r"\s*\*\s*@(\w+)", lines[k])
                    if m:
                        tags.append(m.group(1))
                ranks = [DOC_RANK[t] for t in tags if t in DOC_RANK]
                if ranks != sorted(ranks):
                    rep.add("D3", r, i + 1, "Doxygenのタグ順(@brief,@details,@note,@tparam,@param,@return)")
                i = j + 1
            elif lines[i].strip() == "/*" and i + 1 < len(lines) and re.match(r"\s*\*\s*@brief", lines[i + 1]):
                rep.add("D6", r, i + 1, "@briefのブロックは /** で始める")
                i += 1
            else:
                i += 1

    # F6: ヘッダーが使うstdヘッダーを自分でincludeしているか(.cppは自ヘッダーの分も数える)
    def std_incs(t):
        return set(re.findall(r"#include <([^>]+)>", t))

    by_path = {p: texts[p] for p in files}
    for p in files:
        if p.endswith("CRC32.h"):
            continue
        t = texts[p]
        code = strip_code(t)
        have = std_incs(t)
        if p.endswith(".cpp") and p[:-4] + ".h" in by_path:
            have |= std_incs(by_path[p[:-4] + ".h"])
        for header, rx in STD_TOKENS.items():
            if re.search(rx, code) and header not in have:
                rep.add("F6", rel(p), 1, f"<{header}> をincludeしていない")


def check_misc(root, rep):
    # B2: ParamLoaderの型不一致をassertで止めない
    p = os.path.join(root, "Game", "Source", "Parameter", "ParamLoader.cpp")
    if os.path.exists(p):
        text = open(p, "rb").read().decode("utf-8-sig")
        for i, line in enumerate(text.split("\n"), 1):
            if "K2_ASSERT(false" in line and "VALUE_DIFFER" in line:
                rep.add("B2", "Game/Source/Parameter/ParamLoader.cpp", i, "型不一致をassert(abort)にしない。K2_LOGで知らせて無効値を返す")


def check_clang_format(root, files, rep):
    cf = find_clang_format()
    if cf is None:
        print("[skip] F1: clang-formatが見つからない", file=sys.stderr)
        return
    game = os.path.join(root, "Game")
    if not os.path.exists(os.path.join(game, ".clang-format")):
        print("[skip] F1: Game/.clang-format が無い", file=sys.stderr)
        return
    counts = defaultdict(int)
    for chunk_start in range(0, len(files), 40):
        chunk = files[chunk_start:chunk_start + 40]
        proc = subprocess.run([cf, "--dry-run", "-style=file"] + chunk, cwd=game, capture_output=True, text=True, encoding="utf-8", errors="replace")
        for line in (proc.stderr or "").splitlines():
            m = re.match(r"(.+?):(\d+):\d+: (warning|error):", line)
            if m:
                counts[m.group(1)] += 1
    for path, n in sorted(counts.items()):
        rep.add("F1", os.path.relpath(path, root).replace(os.sep, "/"), 1, f".clang-formatとの差分が{n}件")


def check_sync(root, rep):
    script = os.path.join(root, "tools", "sync_vcxproj.py")
    if not os.path.exists(script):
        print("[skip] G1: tools/sync_vcxproj.py が無い", file=sys.stderr)
        return
    proc = subprocess.run([sys.executable, "-X", "utf8", script, "Game", "--dry-run"], cwd=root, capture_output=True, text=True, encoding="utf-8", errors="replace")
    out = (proc.stdout or "") + (proc.stderr or "")
    if "追加" in out or "削除" in out:
        rep.add("G1", "Game/Game.vcxproj", 1, "ディスクとvcxprojが一致していない。tools/sync_vcxproj.py Game を実行する:\n" + out.strip())
    path = os.path.join(root, "Game", "Game.vcxproj.filters")
    if os.path.exists(path):
        text = open(path, "rb").read().decode("utf-8-sig")
        for m in re.finditer(r'<Filter Include="([^"]+)">', text):
            name = m.group(1)
            used = re.search(r"<Filter>" + re.escape(name) + r"</Filter>", text)
            is_parent = any(other.startswith(name + chr(92)) for other in re.findall(r'<Filter Include="([^"]+)">', text))
            if not used and not is_parent and name.startswith("Source"):
                rep.add("G1", "Game/Game.vcxproj.filters", 1, f"空のフィルター {name}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", help="リポジトリのルート(Game/Source がある所)。省略すると、カレントから上へ探す")
    ap.add_argument("--only", help="調べるチェックIDをカンマ区切りで(例: B1,C2,F1)")
    ap.add_argument("--no-clang-format", action="store_true", help="F1を飛ばす")
    ap.add_argument("--no-sync", action="store_true", help="G1を飛ばす")
    args = ap.parse_args()
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

    root = os.path.abspath(args.root) if args.root else (find_root(os.getcwd()) or find_root(os.path.dirname(os.path.abspath(__file__))))
    if not root or not os.path.isdir(os.path.join(root, "Game", "Source")):
        print("Game/Source が見つからない。--root を指定する", file=sys.stderr)
        return 2
    only = set(args.only.split(",")) if args.only else None
    rep = Report(only)

    files = target_files(root)
    texts = {}
    check_files(root, files, rep, texts)
    check_misc(root, rep)
    if not args.no_clang_format and rep.wants("F1"):
        check_clang_format(root, files, rep)
    if not args.no_sync and rep.wants("G1") and not args.root:
        check_sync(root, rep)

    items = [it for it in rep.items if rep.wants(it[0])]
    order = {"B": 0, "C": 1, "F": 2, "D": 3, "G": 4}
    items.sort(key=lambda it: (order.get(it[0][0], 9), it[0], it[1], it[2]))
    for check_id, path, line, msg in items:
        print(f"{check_id:<4} {path}:{line}  {msg}")
    print(f"\n{len(files)}ファイルを調べた。違反: {len(items)}件", file=sys.stderr)
    return 1 if items else 0


if __name__ == "__main__":
    sys.exit(main())
