@cmake --version
@cd /D "%~dp0"
@rd /s /q CMAKE
@mkdir CMAKE
@cd CMAKE

@IF EXIST "c:\games\Steam\steamapps\common\Deus Ex" @(
	set DEUSEXFOLDER="c:\games\Steam\steamapps\common\Deus Ex"
)

@IF EXIST "C:\Program Files (x86)\Steam\steamapps\common\Deus Ex\" @(
	set DEUSEXFOLDER="C:\Program Files (x86)\Steam\steamapps\common\Deus Ex"
)

@IF EXIST "D:\Games\Steam\steamapps\common\Deus Ex" @(
	set DEUSEXFOLDER="D:\Games\Steam\steamapps\common\Deus Ex"
)

@IF EXIST "F:\Games\Steam\steamapps\common\Deus Ex\System" @(
	set DEUSEXFOLDER="F:\Games\Steam\steamapps\common\Deus Ex\System"
)

@cmake -DEE_DEUSEXFOLDER=%DEUSEXFOLDER%  ^
      -G"Visual Studio 17 2022" -A win32 ..\..

@cd ..
@del /Q DeusExEchelonRenderer.sln 2>/nul
@mklink DeusExEchelonRenderer.sln "%CD%\CMAKE\DeusExEchelonRenderer.sln" 2>/nul

@pause