# BalloonEngine

k2EngineLow の上に作った自作の描画エンジン(静的ライブラリ)です。
`BalloonEngine` フォルダをコピーして、別のプロジェクトでも使えるようにしてあります。

## 入っているもの

| 機能 | 内容 |
|---|---|
| 描画 | `RenderingEngine`(デファードレンダリング、シャドウ、ブルーム、被写界深度、空=`SetSkyCube()`でキューブマップを設定)、`ModelRender`、`SpriteRender`(画像1枚。アルファブレンドのモードを指定できる)、`Light` |
| ImGui | 内蔵済み(`ThirdParty/imgui`)。Game側の設定は不要 |
| フォント | `SDFFontEngine` / `FontRender`。SDFのアトラス(PNG + JSON)で文字を描く。フォントは複数登録でき、`FontRender::SetFont()` で切り替える。拡大しても輪郭が鮮明 |
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

## フォントの使い方

文字は、SDF(符号付き距離場)のアトラス画像(PNG)とグリフ情報(JSON)で描きます。フォントは `SDFFontEngine::MAX_FONT_NUM`(8)個まで登録でき、文字列(`FontRender`)ごとに選べます。
`SDFFontEngine` は `RenderingEngine::Initialize()` / `Finalize()` が初期化・終了するので、Game側は登録と描画だけです。

1. アトラスを作る(msdf-atlas-gen)。

   ```
   msdf-atlas-gen.exe -font xxx.ttf -type sdf -format png -charset charset.txt -imageout xxx.png -json xxx.json -size 48
   ```

   - `charset.txt` は、アトラスに入れる文字のUnicodeコードポイント(10進数)を、空白で区切って並べたもの。フォントに無い文字は、アトラスに入らず、表示されない。
   - `-size` がフォントのem(ピクセル)。`FontRender` の拡大率が1のときの文字の高さになる。
2. `RenderingEngine::Initialize()` の後に、フォントを登録する。戻り値が `FontId`。失敗(ファイルが無い、上限を超えた)したときは `INVALID_FONT_ID`。

   ```cpp
   const FontId fontId = SDFFontEngine::Get().LoadFont("Assets/font/xxx.png", "Assets/font/xxx.json");
   ```

3. `FontRender` で描く。

   ```cpp
   FontRender text;
   text.SetFont(fontId);                // 呼び直すと、フォントを切り替えられる
   text.SetText(L"Hello ∞\n2行目");  // '\n' で改行。255文字まで
   text.SetPosition(0.0f, 0.0f);        // 画面の中心が原点、y上が正
   text.SetPivot(Vector2(0.5f, 0.5f));  // (0, 0)が左下、(1, 1)が右上
   text.SetTextAlign(TextAlign::Center);
   text.Draw(rc);                       // 即時描画
   ```

   `Draw()` は即時描画なので、`RenderingEngine::Execute()` の後(Game側のUIの描画)で呼ぶこと。それより前に呼ぶと、ポストプロセスに上書きされて見えなくなる。
   `SetRotation()` はラジアンで、画面上で時計回りが正(座標の位置が回転の中心)。

## 注意

- **k2Engine(配布の高機能版)とは併用できません。** `ModelRender` などの同名クラスがあり、混ざると混乱します。BalloonEngine を使うプロジェクトでは、k2Engine を参照しないこと。
- シェーダーは `BalloonEngine/shader/` が本体です。Game の `Assets/shader/balloon/` はビルドのたびに上書きコピーされる生成物なので、編集しないこと。
- シェーダーのコピー先を変えたいときは、`BalloonShaderOutDir` を `.props` の Import より前に定義してください。
- ImGui のバージョンは 1.92.9 WIP(上流の改変なし)です。
