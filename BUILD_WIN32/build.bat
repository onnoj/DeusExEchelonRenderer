setlocal enabledelayedexpansion

@cmake --version
@cd /D "%~dp0"
@rd /s /q CMAKE
@mkdir CMAKE
@cd CMAKE

@IF EXIST "%DEUSEXFOLDER%\System" @(
	@echo "Found Deus Ex at %DEUSEXFOLDER%"
) ELSE IF EXIST "c:\games\Steam\steamapps\common\Deus Ex\System" @(
	@set DEUSEXFOLDER="c:\games\Steam\steamapps\common\Deus Ex\System"
	@echo Environment variable DEUSEXFOLDER not set, detected at: !DEUSEXFOLDER!
) ELSE IF EXIST "C:\Program Files (x86)\Steam\steamapps\common\Deus Ex\System" @(
	@set DEUSEXFOLDER="C:\Program Files (x86)\Steam\steamapps\common\Deus Ex\System"
	@echo Environment variable DEUSEXFOLDER not set, detected at: !DEUSEXFOLDER!
) ELSE IF EXIST "D:\Games\Steam\steamapps\common\Deus Ex" @(
	@set DEUSEXFOLDER="D:\Games\Steam\steamapps\common\Deus Ex\System"
	@echo Environment variable DEUSEXFOLDER not set, detected at: !DEUSEXFOLDER!
) ELSE IF EXIST "F:\Games\Steam\steamapps\common\Deus Ex\System" @(
	@set DEUSEXFOLDER="F:\Games\Steam\steamapps\common\Deus Ex\System"
	@echo Environment variable DEUSEXFOLDER not set, detected at: !DEUSEXFOLDER!
) ELSE @(
	@echo Environment Variable DEUSEXFOLDER is either not set, or is invalid.
	@exit 1
)
@set "DEUSEXFOLDER=!DEUSEXFOLDER:"=!"

@cmake -DEE_DEUSEXFOLDER="!DEUSEXFOLDER!"  ^
      -G"Visual Studio 17 2022" -A win32 ..\..

@cd ..
@del /Q DeusExEchelonRenderer.sln 2>/nul
@mklink DeusExEchelonRenderer.sln "%CD%\CMAKE\DeusExEchelonRenderer.sln" 2>/nul

@REM Prompt the user for symlink installation
@echo.
@set /p INSTALL_SYMLINK=^ ^> Do you want to install a symlink to the renderer in the Deus Ex system folder? [Y/N]^ 

@REM Check user's choice
@if /i "%INSTALL_SYMLINK%"=="Y" (
	@del /F "!DEUSEXFOLDER!\DeusExEchelonRenderer.dll"
    @mklink "!DEUSEXFOLDER!\DeusExEchelonRenderer.dll" "%CD%\..\BIN\USER\DeusExEchelonRenderer.dll" && @(
	    @echo.
		@echo Symlink created successfully.
		@echo You'll still want to build the 'INSTALL' target to also copy over the config files, at least once.
		@echo.
	)
	
	@del /F "!DEUSEXFOLDER!\DeusExEchelonRendererWindowDrv.dll"
    @mklink "!DEUSEXFOLDER!\DeusExEchelonRendererWindowDrv.dll" "%CD%\..\BIN\USER\DeusExEchelonRendererWindowDrv.dll" && @(
	    @echo.
		@echo Symlink created successfully.
		@echo You'll still want to build the 'INSTALL' target to also copy over the config files, at least once.
		@echo.
	)
	
	@del /F "!DEUSEXFOLDER!\deusexwindowdrv.exe"
    @mklink "!DEUSEXFOLDER!\deusexwindowdrv.exe" "!DEUSEXFOLDER!\DeusEx.exe" && @(
	    @echo.
		@echo Symlink created successfully.
		@echo You'll still want to build the 'INSTALL' target to also copy over the config files, at least once.
		@echo.
	)
)
@pause