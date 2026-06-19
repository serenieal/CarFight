@echo off
REM CarFight shared Unreal Engine environment guard.
REM Version: v1.0.0
REM Changelog:
REM - v1.0.0: Lock build and editor tools to D:\UnrealEngine_Source.

REM Repository root folder shared by CarFight tools.
for %%I in ("%~dp0..") do set "CARFIGHT_ROOT_DIR=%%~fI"

REM Unreal project folder shared by CarFight tools.
set "CARFIGHT_UE_DIR=%CARFIGHT_ROOT_DIR%\UE"

REM CarFight Unreal project file shared by CarFight tools.
set "CARFIGHT_UPROJECT=%CARFIGHT_UE_DIR%\CarFight_Re.uproject"

REM Required Unreal Engine source build root for this project.
set "CARFIGHT_ENGINE_ROOT=D:\UnrealEngine_Source"

REM Unreal Editor executable that must be used for this project.
set "CARFIGHT_EDITOR_EXE=%CARFIGHT_ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

REM Unreal Editor commandlet executable that must be used for this project.
set "CARFIGHT_EDITOR_CMD_EXE=%CARFIGHT_ENGINE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM Unreal Build Tool batch file that must be used for this project.
set "CARFIGHT_BUILD_BAT=%CARFIGHT_ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat"

REM Environment guard result used by caller batch files.
set "CARFIGHT_ENV_READY="

if not exist "%CARFIGHT_UPROJECT%" (
	echo [ERROR] CarFight project file was not found:
	echo %CARFIGHT_UPROJECT%
	exit /b 10
)

if not exist "%CARFIGHT_EDITOR_EXE%" (
	echo [ERROR] Required UnrealEditor.exe was not found:
	echo %CARFIGHT_EDITOR_EXE%
	exit /b 11
)

if not exist "%CARFIGHT_BUILD_BAT%" (
	echo [ERROR] Required Build.bat was not found:
	echo %CARFIGHT_BUILD_BAT%
	exit /b 12
)

powershell -NoProfile -ExecutionPolicy Bypass -Command "$project = Get-Content -Raw -Encoding UTF8 -LiteralPath $env:CARFIGHT_UPROJECT; $assoc = (ConvertFrom-Json $project).EngineAssociation; $reg = Get-ItemProperty -Path 'HKCU:\Software\Epic Games\Unreal Engine\Builds' -ErrorAction Stop; $prop = $reg.PSObject.Properties[$assoc]; if (-not $prop) { Write-Host '[ERROR] EngineAssociation is not registered:' $assoc; exit 20 }; $actual = ($prop.Value.TrimEnd('\','/') -replace '\\','/'); $expected = ($env:CARFIGHT_ENGINE_ROOT.TrimEnd('\','/') -replace '\\','/'); if ($actual -ne $expected) { Write-Host '[ERROR] EngineAssociation points to a different engine.'; Write-Host 'Expected:' $expected; Write-Host 'Actual  :' $actual; exit 21 }"
if errorlevel 1 (
	exit /b %errorlevel%
)

set "CARFIGHT_ENV_READY=1"
exit /b 0
