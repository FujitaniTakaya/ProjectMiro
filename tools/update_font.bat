@echo off
chcp 65001 >nul
rem SDFフォントのアトラス(PNG + JSON)を作るバッチ。
rem
rem   update_font.bat                      Game\Assets\font の、コミットしているフォント全てのアトラスを作る
rem   update_font.bat <ttfのパス>          指定したフォント1つのアトラスを作る
rem                                        (Game\Assets\font\local\<ttfのファイル名>.png と .json ができる。
rem                                         local フォルダは git に入れない。Windows付属のフォントなど、再配布できないフォントの確認用)
rem
rem アトラスの文字は tools\make_charset.py の範囲(日本語の標準範囲 + ∞など)。
rem 文字サイズ(-size)を変えたら、使う側(FontRender)の拡大率も変わる。(1.0 で -size の高さ)
cd /d "%~dp0.."

set SIZE=48
set CHARSET=tools\charset.txt

where python >nul 2>nul
if %errorlevel%==0 (
    python -X utf8 tools\make_charset.py "%CHARSET%"
) else (
    py -X utf8 tools\make_charset.py "%CHARSET%"
)
if %errorlevel% neq 0 (
    echo [エラー] charset の生成に失敗しました。
    pause
    exit /b 1
)

if not "%~1"=="" (
    call :generate "%~1" "Game\Assets\font\local"
    goto :done
)

rem コミットしているフォントは、ここに足す。
call :generate "Game\Assets\font\rounded-mplus-1c-heavy.ttf" "Game\Assets\font"

:done
echo.
pause
exit /b 0


:generate
set TTF=%~1
set OUTDIR=%~2
set NAME=%~n1
if not exist "%OUTDIR%" mkdir "%OUTDIR%"
echo [アトラス生成中: %NAME% ... 少し時間がかかります]
tools\msdf-atlas-gen.exe -font "%TTF%" -type sdf -format png -charset "%CHARSET%" -imageout "%OUTDIR%\%NAME%.png" -json "%OUTDIR%\%NAME%.json" -size %SIZE%
if %errorlevel% neq 0 (
    echo [エラー] %NAME% のアトラス生成に失敗しました。
    exit /b 1
)
echo [完了] %OUTDIR%\%NAME%.png / %NAME%.json を更新しました。
echo        フォントに無い文字は Missing ... codepoints と表示されます。その文字は表示されません。
exit /b 0
