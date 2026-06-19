@echo off
REM File: SetVehicleSingleDefaults.bat
REM Version: v1.1.0
REM Changelog:
REM - v1.1.0: Match SetVehicleSingleDefaults.py v1.1.0 after removing network diagnostic flags.
REM - v1.0.0: Run the Python commandlet that saves BP_CFVehiclePawn single-player defaults.

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
set "SCRIPT_PATH=%TOOLS_DIR%SetVehicleSingleDefaults.py"

echo [CarFight] Synchronizing BP_CFVehiclePawn single-player defaults.
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT_PATH%" -unattended -nop4 -NoSound -utf8output

exit /b %ERRORLEVEL%
