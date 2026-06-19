@echo off
REM File: CleanSingleMap.bat
REM Version: v1.0.0
REM Changelog:
REM - v1.0.0: Run the Python commandlet that removes CFNetSmooth test actors from TestMap.

setlocal

REM Tools directory that contains this batch file.
set "TOOLS_DIR=%~dp0"

REM CarFight project root directory.
set "PROJECT_ROOT=%TOOLS_DIR%.."

REM CarFight Unreal project file path.
set "UPROJECT=%PROJECT_ROOT%\UE\CarFight_Re.uproject"

REM Unreal Editor commandlet executable path.
set "UE_EDITOR_CMD=D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM Unreal Python script path.
set "SCRIPT_PATH=%TOOLS_DIR%CleanSingleMap.py"

echo [CarFight] Cleaning TestMap for single-player rollback.
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT_PATH%" -unattended -nop4 -NoSound -utf8output

exit /b %ERRORLEVEL%
