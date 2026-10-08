# Miro コードの矛盾・統一 一覧(2026-10-09 監査)

`Game/Source` と `Game/Application.*`、`Game/IObject.h` を、3つの観点(マネージャー/ハンドル、書式/命名、パラメーター/ビルド登録)で調査して見つかったものと、その決定。

- **状態**: 修正済み / 現状維持(意図して残した) / 未確認(調査報告のみ。私が実コードで確認していない)
- **検出**: `check.py` のチェックID。`—` は機械検出できないもの。
- 行番号は監査時点のもの。

---

## B. 動作に影響する点

| ID | 矛盾 | 決定したルール | 状態 | 検出 |
|---|---|---|---|---|
| B1 | デバッグ切替のマクロが3種類。ホットリロードは`_DEBUG`、デバッグUIは`BALLOON_IMGUI_ENABLED`、assert/logは`K2_DEBUG`。Preview|x64は`K2_DEBUG;NDEBUG`で`_DEBUG`が無いため、F3の画面だけ出てホットリロードが効かなかった | ImGuiは`BALLOON_IMGUI_ENABLED`。開発用の処理・`K2_LOG`・`K2_ASSERT`は`K2_DEBUG`(Debug+Previewで有効、Releaseで無効)。Miroのコードで`_DEBUG`は使わない | 修正済み | B1 |
| B2 | `ParamLoader`のjson型不一致が`K2_ASSERT(false)`でabort。Layout/UIAnimationは「壊れたjsonは直前の値を保つ」設計なので、ホットリロード中の書き間違いでゲームが落ちた | 回復できる不正データは、キー名付き`K2_LOG`+無効値を返す。assertで止めるのは、プログラマーのミス(未登録ID、ハンドル枯渇)だけ。Beastはassertだったが意図した差分 | 修正済み | B2 |
| B3 | `SceneManager::CreateScene`が未登録IDで失敗すると、`LoadingScene`のまま暗転して止まる(Releaseではassertが消える) | 失敗したらシーン無しで`FadingIn`へ進める。assertは残す | 修正済み | — |
| B4 | `HotReloadManager`は大文字小文字・`/`と`\`の違いを別ファイル扱い(Windowsは区別しない)。`UIAnimationParameter::Load`の重複判定は生文字列の比較 | パスの同一判定は`HotReloadManager::NormalizePath`(`/`、`./`、ASCIIの大文字小文字を無視)。小文字化はwide文字列で行う(Shift-JISの2バイト目を壊さない) | 修正済み | — |
| B5 | エンジン側`k2EngineLow/dbg/MyAssert.h`の`va_start(va, flag)`が、最後の名前付き引数`line`ではない(未定義動作) | Miroのassertは書式引数を使わないので顕在化しない。エンジン側は触らない | 現状維持 | — |

## C. 方式・規約の食い違い

| ID | 矛盾 | 決定したルール | 状態 | 検出 |
|---|---|---|---|---|
| C1 | シングルトンが、`CreateInstance/DestroyInstance`(Sound/Effect/ParamHolder/Fade/SceneManager)と、関数内static`Get()`(Timer/HotReload/UIAnimationParameter)の2方式 | **コンストラクタで起動時の処理をするものだけ**Create/Destroy(エンジン資源を持つSound/Effect/Fade、最初のシーンを作るSceneManager、全jsonを読むParamHolder)。それ以外は関数内static。統一はせず、各ヘッダーのCreateInstanceの`@details`に理由を書いた | 修正済み(明文化) | — |
| C2 | メンバーのデフォルト初期化子(`m_x = nullptr;`)が残っていた(`EffectManager.h`のEffectEntry、`StateMachineBase.h`のIState/`m_currentState`、`IObject.h`)。`StateMachineBase`は`m_currentState`が二重初期化で`m_states`が初期化リストに無かった | メンバーはコンストラクタの初期化リストで初期化する。ネストした補助structも、初期値が要るならコンストラクタを持たせる | 修正済み | C2 |
| C3 | `Fade`と`ui::UISprite`が、同じ理由(`SpriteRender`にアルファブレンドが無い)で`Sprite`を包んでいた。シェーダーパス定数も重複 | 画像1枚を描く用途は`ui::UISprite`を使う | 修正済み | — |
| C4 | `HotReloadHandle`だけマネージャーのヘッダー内で定義、払い出しのオーバーフロー検査も無い | `using XHandle = uint32_t`と`INVALID_X_HANDLE = 0xffffffff`は軽量ヘッダー`XHandle.h`に置く。払い出しの枯渇は`K2_ASSERT`で検査する(Sound/Effect/HotReload) | 修正済み | — |
| C5 | ハンドルが4つとも`uint32_t`の別名で相互に代入できる。世代付き(index+generation)はTimerだけ。Sound/Effectはファイルスコープのグローバルカウンタ(Soundは全グループ共通でmapはグループごと) | 新規のハンドルは、Timerと同じindex+generation方式 | 現状維持(既存は変更しない) | — |
| C6 | ID方式が3系統: `Hash32`(CRC32のuint32)、`uint8_t`のenum(`EnSoundID`/`EnEffectKind`/`EnParamID`、区切りは`Max, None = Max`)、`uint32_t`ハンドル。登録の重複検査も、SoundはコンパイルタイムのIsUnique、Effectはサイズのstatic_assert、Paramは実行時assertと不揃い | 新規の項目ごとにenumを増やさない(小さな閉じた集合だけenum)。既存は変更しない | 現状維持 | — |
| C7 | API語彙のばらつき: `Init`/`Initialize`/`Setup`、`Play`と`PlayAnimation`、`IsDraw`と`IsVisible`、全消去が`StopAll`/`StopAllEffect`/`DestroyAllTimer`、`Stop`の意味(Timerは一時停止、Sound/Effectは終了) | Beast由来の語彙は変えない。新規のAPIは、近いモジュールの語彙に合わせる | 現状維持 | — |
| C8 | パラメーター読み込みが2系統。`ParamHolder`(`EnParamID`、登録0件で未使用)と、`Layout`/`UIAnimationParameter`が`ParamLoader`+`HotReloadManager`を直接使用。後者はF3のParamDebugUIに出ず、Reloadもできない | 当面そのまま(ParamHolderは必要になった時に使う) | 現状維持 | — |
| C9 | jsonスキーマの差: Layoutの要素ID`name`とアニメーションの`key`、回転が`Layout`はスカラー(Z軸の度)で`ParamLoader::ToRotation`は配列。Beastの`{"x":..,"y":..}`形式のVector4は読めない(配列のみ。意図) | 変更しない(jsonがリポジトリにまだ無いので、変えるなら今が安い)。Beastのjsonを移すときは変換する | 現状維持 | — |
| C10 | `RequesutScene`のタイポ(Beastから移植)が公開APIに残っていた | `RequestScene`に改名。Beastのシーンを移植するときは、その1語を置き換える | 修正済み | C10 |
| C11 | `AIControllerConfig`/`PlayerController`は定数を直書き(Beastではjson管理だった値) | 暫定。jsonに繋ぐときにParamHolderかParamLoaderで読む | 現状維持 | — |
| C12 | シーン切り替えで、Timer/Effect/Soundを解放する処理が無い(`TimerManager.h`のドキュメントは「シーン切り替えで`DestroyAllTimer`を呼ぶ」と書くが、呼ぶ所が無い)。`AttachEffect`は生ポインタを保持する | シーンが実装されるときに決める | 現状維持(未実装) | — |

## F. 書式

| ID | 矛盾 | 決定したルール | 状態 | 検出 |
|---|---|---|---|---|
| F1 | `Game/.clang-format`との差分(CRC32.h 280、ParamLoader.cpp 54、Curve.h 42、AIController.cpp 32、ParamHolder.h 22、IScene.h 18ほか。Sound/Timer/Effectは0) | 対象は`Game/Source/**`、`Application.*`、`IObject.h`。`clang-format --dry-run`が0件であること。`Game.*`、`main.cpp`、`system/`、`stdafx.*`は対象外(テンプレート由来) | 修正済み | F1 |
| F2 | 文字コード: `CRC32.h`がShift-JIS(BOMなし)。MSVCはBOMなしUTF-8をcp932として読む(C4819) | UTF-8(BOM付き) | 修正済み | F2 |
| F3 | 改行: `UIAnimation`の5ファイルが作業ツリーでLF(`autocrlf=true`なのでコミットには影響しない)。`StateMachineBase.{h,cpp}`は末尾改行なし | CRLF、末尾改行あり | 修正済み | F3 |
| F4 | インデント: `CRC32.h`と`Application.*`、`IObject.h`がタブ | 4スペース(Microsoftスタイル)。`main.cpp`/`system/`は対象外 | 修正済み | F4 |
| F5 | include表記: 同じモジュール内で`"Source/Parameter/X.h"`のフルパスと素の名前が混在。順序・グルーピングも不揃い | 同じディレクトリは`"X.h"`、他モジュールは`"Source/<Module>/X.h"`、子ディレクトリは相対(`"Animation/X.h"`)。`stdafx.h`→空行→自ヘッダー→std→プロジェクトの順(`IncludeBlocks: Regroup`が整える) | 修正済み | F5 |
| F6 | ヘッダーがPCH頼みで`<memory>`/`<utility>`/`<cstdint>`/`<algorithm>`などをincludeしていない | ヘッダーは使うstdヘッダーを自分でincludeする | 修正済み | F6 |
| F7 | `} // namespace app`の閉じコメントがController/MoveBehavior/Scene/Parameterに無かった。内部namespace名が`_internal`と`detail` | 閉じコメントあり(clang-formatが付ける)。内部は`detail` | 修正済み | F7 |
| F8 | `appState(Idle)`のようなセミコロン無しのマクロを、clang-formatが次の`public:`と結合して`public : explicit ...`に崩した | `Game/.clang-format`の`StatementMacros`に`appState`/`appScene`を登録。セミコロン無しのマクロを新設したら、ここにも足す | 修正済み | F8 |
| F9 | 全角の括弧、範囲記号が`～`(U+FF5E)と`〜`(U+301C)で混在 | 半角の括弧。範囲は`〜`(U+301C) | 修正済み | F9 |
| F10 | 波括弧なしの`if`/`for`(主にParameter/)。`{ VALUE_DIFFER; return invalid; }`の1行書き | 常に波括弧を付ける。(一度だけ`InsertBraces: true`で整形した。`.clang-format`には入れていない) | 修正済み | F10 |
| F11 | 空のコンストラクタ/デストラクタの書き方が3通り(`{`と`}`を別行、`{}`だけの行、インライン`{}`)。clang-formatは全て通す | 揃えない | 現状維持 | — |
| F12 | enumの区切りが`Max`(`None = Max`)と`Count`(`CharacterInput.h`)、`En`接頭辞の有無、基底型`: uint8_t`の有無 | Miro起源のモジュール単位のenumは`En`+`uint8_t`+`Max`。Beast由来は変えない | 現状維持 | — |

## D. ドキュメント

| ID | 矛盾 | 決定したルール | 状態 | 検出 |
|---|---|---|---|---|
| D1 | `Application.h`のPreUpdate説明が、Sound/Effectの回収だけを書いていた(HotReloadとTimerも呼ぶ) | `Application`に呼び出しを足したら、説明も直す | 修正済み | — |
| D2 | `SoundManager.h`の例が存在しない`EnSoundID::Damage`。`EnSoundID`の項目が「グループ名」と読める | `EnSoundID`のBGM/SE/Voiceは音を足すまでの仮の項目、とドキュメントに書いた | 修正済み | — |
| D3 | Doxygenのタグ順が不揃い(`@param`が`@details`より前など) | `@brief`、`@details`、`@note`、`@tparam`、`@param`、`@return`の順。日本語、`@param[out] name`の書き方 | 修正済み | D3 |
| D4 | `StateMachineBase.h`の`@param stateID`が実引数名`stateMachine`と違う。`IScene.h`が`id[out]` | 実引数名に合わせる。`@param[out] id` | 修正済み | — |
| D5 | jsonパスの例が`Assets/Param/`、`Assets/parameter/UI/`、`Assets/xxx/`の3通り | `Assets/parameter/<小文字のモジュール>/<Pascal>.json` | 修正済み | — |
| D6 | `/*`で始まる`@brief`ブロック(`StateMachineBase.h`) | Doxygenは`/**`で始める | 修正済み | D6 |

## G. ビルド登録・履歴

| ID | 矛盾 | 決定 | 状態 | 検出 |
|---|---|---|---|---|
| G1 | `Game.vcxproj`/`.filters`はディスクと一致。空の`Source\Core`フィルターだけ残っていた | 新規ファイルは`tools/sync_vcxproj.py Game`で登録。`--dry-run`が差分なしであること。空フィルターは手で消す(スクリプトは掃除しない) | 修正済み | G1 |
| G2 | `BalloonEngine.vcxproj`にimguiが7行あるのに`BalloonEngine.vcxproj.filters`は0行(確認済み)。`sync_vcxproj.py`は、vcxprojにあって`.filters`に無い項目を検出できず、空フィルターも掃除しない(スクリプトの挙動は調査報告のみ、未確認) | 触っていない | 一部未確認 | — |
| G3 | 古い6コミットに`Co-Authored-By`が無い/件名の形式が違う | 履歴は書き換えない。新しいコミットは「〜を追加し、〜できるようにする」形式 | 現状維持 | — |
| G4 | 一部の中間コミット(9023c93、f585ec5、21e31f1)で、ファイルがvcxprojに未登録(`fa40e96`で解消) | 以後は各コミットでvcxprojを同期する | 現状維持 | — |

---

## 揃っていた点(変更なし)
`#pragma once`、`enum class`のみ使用、`override`付与、メンバーは`m_`+camelCase、メソッドはPascalCase、assertは`K2_ASSERT`のみ、経過時間は`g_gameTime->GetFrameDeltaTime()`から読む(1/60の直書きなし)、日本語のDoxygenコメントと`@file`/`@brief`ヘッダー、`[[nodiscard]]`/`noexcept`/`shared_ptr`は不使用、所有は`unique_ptr`で非所有は生ポインタ。
