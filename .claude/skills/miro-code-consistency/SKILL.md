---
name: miro-code-consistency
description: ProjectMiro(Game/Source のC++)の記述・実装方式を揃えるためのスキル。新しいクラスやファイルを書くとき、コードを見直すとき、コミット前に「規約に合っているか」「矛盾はないか」を確認するときに使う。デバッグマクロ(K2_DEBUG/BALLOON_IMGUI_ENABLED)、シングルトンの方式、メンバー初期化、include、Doxygen、文字コード、clang-format、jsonパス、ハンドルなど。
user-invocable: true
---

# Miro コードの一貫性

`Game/Source` のコードを、決めた規約に揃えるためのスキル。2026-10-09 の監査で見つかった矛盾と、その決定は [contradictions.md](contradictions.md) にまとめてある(状態: 修正済み/現状維持/未確認)。

## いつ使うか
- 新しいクラス・ファイルを書く前(下の「書くときのルール」を読む)
- コードを見直す、またはコミットする前(`check.py`を実行する)
- 「ここは前に揃えたはず」「どちらの方式が正しい」と迷ったとき(contradictions.md を引く)

## 確認の手順
1. リポジトリのルートで実行する。違反が出たら、IDを contradictions.md で引いて直す。
   ```
   python -X utf8 .claude/skills/miro-code-consistency/check.py
   python -X utf8 .claude/skills/miro-code-consistency/check.py --only B1,C2,F1   # 一部だけ
   ```
   違反なしで終了コード0。調べるのは `Game/Source/**`、`Game/Application.*`、`Game/IObject.h`。何も書き換えない。
2. 書式(F1)は `clang-format -i` で直せる。場所は `C:\Program Files\Microsoft Visual Studio\2026\VC\Tools\Llvm\x64\bin\clang-format.exe`、`Game/` から `-style=file` で実行する。
3. 新しい.h/.cppを足したら、`python -X utf8 tools/sync_vcxproj.py Game`(先に`--dry-run`)で`Game.vcxproj`/`.filters`に登録する(G1)。
4. ビルドは PowerShell から `Game.sln` を `Debug`・`Preview`・`Release`(x64)の3構成で通す。`K2_DEBUG`の有無で挙動が変わるので、Previewも通すこと。
5. `check.py`で検出できない項目(下の「機械検出できないもの」)は、自分で見る。

## 書くときのルール(要約)
- **デバッグマクロ(B1)**: ImGuiは`BALLOON_IMGUI_ENABLED`、開発用の処理・`K2_LOG`・`K2_ASSERT`は`K2_DEBUG`で囲む。`_DEBUG`は使わない。
- **エラーの扱い(B2)**: 回復できる不正データ(jsonの型違いなど)は`K2_LOG`+無効値。assertで止めるのはプログラマーのミス(未登録ID、枯渇)だけ。
- **シングルトン(C1)**: コンストラクタで起動時の処理(エンジン資源、全ファイルの読み込み、最初のシーンの生成)をするものだけ`CreateInstance/DestroyInstance`(`Application`で生成・破棄、`@details`に理由を書く)。それ以外は関数内static`Get()`+`Noncopyable`+private constructor。毎フレームの更新は`Application::PreUpdate()`に1行足し、時間は`g_gameTime->GetFrameDeltaTime()`を1回読む。
- **ハンドル(C4, C5)**: `XHandle.h`(軽量ヘッダー)に`using XHandle = uint32_t`と`INVALID_X_HANDLE = 0xffffffff`。新規は index(下位16bit)+generation(上位16bit)で、枯渇は`K2_ASSERT`。項目ごとのenumは作らない。
- **メンバー初期化(C2)**: コンストラクタの初期化リスト(宣言順)。`int m_x = 0;`のようなデフォルト初期化子は使わない。ネストしたstructも、初期値が要るならコンストラクタを持たせる。
- **画像1枚の描画(C3)**: `ui::UISprite`を使う。`SpriteRender`はアルファブレンドできない。
- **include(F5, F6)**: `stdafx.h`→空行→自ヘッダー→std`<...>`→プロジェクト`"..."`(clang-formatが整える)。同じディレクトリは`"X.h"`、他モジュールは`"Source/<Module>/X.h"`。ヘッダーは使うstdヘッダーを自分でincludeする(PCH頼みにしない)。
- **Doxygen(D3)**: 日本語。`@brief`→`@details`→`@note`→`@tparam`→`@param`→`@return`の順。`/**`で始める。`@param[out] name`。半角の括弧、範囲は`〜`。
- **ファイル(F1〜F4, F7)**: UTF-8(BOM付き)、CRLF、末尾改行あり、4スペース。`Game/.clang-format`で差分0。閉じは`} // namespace app`。セミコロン無しのマクロ(`appState`など)を新設したら`.clang-format`の`StatementMacros`に足す。
- **jsonのパス(D5)**: `Assets/parameter/<小文字のモジュール>/<Pascal>.json`。同じファイルかどうかは`HotReloadManager::NormalizePath`で比べる。
- **命名**: メンバーは`m_`+camelCase、メソッドはPascalCase、定数はUPPER_SNAKE、enumは`enum class`、モジュール単位は`En`+`: uint8_t`+`Max`(`None = Max`)。
- **触らないもの**: `k2EngineLow`(エンジン)、`Game.*`・`main.cpp`・`system/`(テンプレート由来、`main.cpp`/`system`はShift-JIS)。

## 機械検出できないもの(自分で見る)
- C1: 新しいマネージャーが、上のシングルトンの規則に合っているか(理由が`@details`に書かれているか)
- C3〜C9, C11, C12, F11, F12: contradictions.md の「現状維持」。既存は変えず、新規は近いモジュールの語彙・方式に合わせる
- D1, D2, D4, D5: 説明・例・`@param`名が、実際のコードと合っているか(呼び出しを足したら説明も直す)
- B3, B4: 失敗したときに止まらないか、パスの比較が正規化されているか

## 注意(ハマりやすい点)
- Bashツールは、コマンド中の`\\n`や`\\x5c`を、実際の改行や1文字のバックスラッシュに変えてしまう。ソースの文字列リテラルを書き換えるときは、Pythonで`chr(92)`を使い、書いた後に`ascii()`で中身を確認する。
- Write/Editで作ったファイルはBOMもCRLFも付かないことがある。新規ファイルは、BOM+CRLFにそろえてからビルドする(BOMが無いとC4819)。
- clang-formatをWindows版で別の設定ファイルに向けるときは、Windowsのパスで書く(`-style="file:C:\...\.clang-format"`)。
- 一括で整形・置換したら、整形前のコピーと`diff -w`で、意図しない変更が無いか確認する。
