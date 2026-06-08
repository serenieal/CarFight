@echo off
REM CarFight VehicleNetDebug log run script - Replicate Movement FALSE test.
REM Version: v0.1.0

set "ROOT_DIR=D:\Work\CarFight_git"
set "UE_DIR=%ROOT_DIR%\UE"
set "LOG_DIR=%ROOT_DIR%\RuntimeLogs\VehicleNet\B_RepMoveFalse"
set "SERVER_EXE=%UE_DIR%\Binaries\Win64\CarFight_ReServer.exe"
set "EDITOR_EXE=D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe"
set "UPROJECT=%UE_DIR%\CarFight_Re.uproject"
set "MAP=/Game/Maps/TestMap"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

echo [CarFight] RepMove FALSE log folder:
echo %LOG_DIR%
echo.
echo [CHECK] Before running this file, set BP_CFVehiclePawn Replicate Movement = FALSE, Compile, Save.
echo.
pause

start "CF Server FALSE" cmd /k ""%SERVER_EXE%" %MAP% -log -Abslog="%LOG_DIR%\Server.log""
timeout /t 5 /nobreak >nul

start "CF Client1 FALSE" cmd /k ""%EDITOR_EXE%" "%UPROJECT%" 127.0.0.1 -game -windowed -ResX=1280 -ResY=720 -WinX=20 -WinY=80 -log -Abslog="%LOG_DIR%\Client1.log""
timeout /t 3 /nobreak >nul

start "CF Client2 FALSE" cmd /k ""%EDITOR_EXE%" "%UPROJECT%" 127.0.0.1 -game -windowed -ResX=1280 -ResY=720 -WinX=1320 -WinY=80 -log -Abslog="%LOG_DIR%\Client2.log""

echo.
echo [CarFight] Server, Client1, Client2 launched.
echo [CarFight] After testing, close all three windows and run Tools\ExtractNetLog.bat.
pause
