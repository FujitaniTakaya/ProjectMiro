# BalloonEngine

k2EngineLow の上に作った自作の描画エンジン(静的ライブラリ)です。
`BalloonEngine` フォルダをコピーして、別のプロジェクトでも使えるようにしてあります。

## 入っているもの

| 機能 | 内容 |
|---|---|
| 描画 | `RenderingEngine`(デファードレンダリング、シャドウ、ブルーム、被写界深度)、`ModelRender`、`SpriteRender`、`Light` |
| ImGui | 内蔵済み(`ThirdParty/imgui`)。Game側の設定は不要 |
| パラメータ調整UI | `ui/ParameterUI`。ライト、ブルーム、被写界深度などを実行中に調整できる(F1で表示・非表示) |
| シェーダー | `shader/`(ビルド時に Game の `Assets/shader/balloon/` へ自動コピー) |
| 設定 | `BalloonEngine.props`(include パス、ライブラリ、C++17、シェーダーのコピー) |

名前空間は `nsBalloonEngine` です。

## 前提

`BalloonEngine` と同じ階層に、次のフォルダがあること。

```
プロジェクト/
  BalloonEngine/   ← このフォルダ
  k2EngineLow/     ← 必須(改造しないこと)
  exlib/           ← ビルドした .lib の出力先(無ければ作られる)
  Game/            ← 使う側のプロジェクト
```

## 別のプロジェクトで使う手順

1. `BalloonEngine/` フォルダを、上の構成になる場所にコピーする。
2. ソリューションに `BalloonEngine/BalloonEngine.vcxproj` を追加する。
   (Visual Studio の「既存のプロジェクトを追加」)
3. Game のプロジェクトを次のように変更する。
   - `Game.vcxproj` の `Microsoft.Cpp.props` の Import より後ろに、次の1行を追加する。
     ```xml
     <Import Project="..\BalloonEngine\BalloonEngine.props" />
     ```
   - `BalloonEngine` へのプロジェクト参照を追加する。
     (Visual Studio の「参照の追加」から BalloonEngine を選ぶ)
4. Game の `stdafx.h` を次のようにする。
   ```cpp
   #include "k2EngineLowPreCompile.h"
   using namespace nsK2EngineLow;

   #include "BalloonEngine.h"
   using namespace nsBalloonEngine;
   ```
5. Game の `main.cpp` で、エンジンを初期化・実行・終了する。
   ```cpp
   // 初期化(NewGO の後)
   RenderingEngine::Get().Initialize();

   // ゲームループ
   g_engine->BeginFrame();
   g_engine->ExecuteUpdate();
   g_engine->ExecuteRender();
   RenderingEngine::Get().Execute();
   g_engine->EndFrame();

   // 終了(エンジンを破棄する前に)
   RenderingEngine::Get().Finalize();
   ```

## ImGui の使い方

`BalloonEngine.h` が `imgui.h` を読み込むので、Game側から `ImGui::` をそのまま使えます。
ウィンドウプロシージャへの接続や、毎フレームの開始・描画は、エンジンが自動で行います。

```cpp
#ifdef BALLOON_IMGUI_ENABLED
    ImGui::Begin("Parameter");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
#endif
```

- パラメータ調整UI(`BalloonEngine` という名前のウィンドウ)は、ImGui が有効なとき自動で表示されます。Game側のウィンドウには、別の名前を付けてください。
- ImGui は **Release 以外**で有効です(`BALLOON_IMGUI_ENABLED` が定義される)。
- Release では ImGui が初期化されないので、`ImGui::` を呼ぶコードは必ず `#ifdef BALLOON_IMGUI_ENABLED` で囲むこと。
- ImGui のウィンドウ配置は、実行フォルダの `imgui.ini` に保存されます(gitignore 済み)。

## 注意

- **k2Engine(配布の高機能版)とは併用できません。** `ModelRender` などの同名クラスがあり、混ざると混乱します。BalloonEngine を使うプロジェクトでは、k2Engine を参照しないこと。
- シェーダーは `BalloonEngine/shader/` が本体です。Game の `Assets/shader/balloon/` はビルドのたびに上書きコピーされる生成物なので、編集しないこと。
- シェーダーのコピー先を変えたいときは、`BalloonShaderOutDir` を `.props` の Import より前に定義してください。
- ImGui のバージョンは 1.92.9 WIP(上流の改変なし)です。
