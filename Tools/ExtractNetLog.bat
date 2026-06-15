@echo off
REM CarFight vehicle network log extraction script.
REM Version: v0.9.0
REM Changelog:
REM - v0.9.0: Remove deleted experiment log extraction targets and keep VehicleNetDebug baseline logs only.
REM - v0.8.4: Extract VehicleRemoteProxyCollision logs for hidden remote proxy collision diagnosis.
REM - v0.8.3: Extract VehicleAutonomousReconcile logs for local owner server sync diagnosis.
REM - v0.8.2: Extract VehicleAutonomousPhysicsPolicy logs for local controlled vehicle shake diagnosis.
REM - v0.8.1: Extract VehicleRemoteVisualShellChild logs separately for visible component transform diagnosis.
REM - v0.8.0: Extract VehicleRemoteVisualShell and include it in combined client network logs.
REM - v0.7.0: Extract server VehicleNetDebug, VehicleNetStateBase, and combined server network logs.
REM - v0.6.0: Extract server VehicleAuthorityGuard logs and spawn context.
REM - v0.5.0: Extract VehicleRemoteProxyPhysics and include it in combined vehicle network logs.
REM - v0.4.0: Extract VehicleTransformAudit and include it in combined vehicle network logs.
REM - v0.3.0: Extract VehicleRemoteInterp and include it in combined vehicle network logs.
REM - v0.2.0: Extract VehicleNetStateBase and combined vehicle network logs.
REM - v0.1.0: Extract VehicleNetDebug logs.

REM Repository root folder used by this extraction script.
set "ROOT_DIR=D:\Work\CarFight_git"

REM Vehicle network log base folder.
set "BASE_DIR=%ROOT_DIR%\RuntimeLogs\VehicleNet"

echo [CarFight] Extract vehicle network baseline log lines.
echo.

for %%T in (A_RepMoveTrue B_RepMoveFalse) do (
    if exist "%BASE_DIR%\%%T\Server.log" (
        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Server.log" > "%BASE_DIR%\%%T\Server_VehicleNetDebug.txt"
        echo Wrote %%T\Server_VehicleNetDebug.txt

        findstr /C:"VehicleNetDebug" /C:"LogCFMPGameMode" "%BASE_DIR%\%%T\Server.log" > "%BASE_DIR%\%%T\Server_VehicleNetAll.txt"
        echo Wrote %%T\Server_VehicleNetAll.txt
    ) else (
        echo Missing %%T\Server.log
    )

    if exist "%BASE_DIR%\%%T\Client1.log" (
        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client1.log" > "%BASE_DIR%\%%T\Client1_VehicleNetDebug.txt"
        echo Wrote %%T\Client1_VehicleNetDebug.txt

        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client1.log" > "%BASE_DIR%\%%T\Client1_VehicleNetAll.txt"
        echo Wrote %%T\Client1_VehicleNetAll.txt
    ) else (
        echo Missing %%T\Client1.log
    )

    if exist "%BASE_DIR%\%%T\Client2.log" (
        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client2.log" > "%BASE_DIR%\%%T\Client2_VehicleNetDebug.txt"
        echo Wrote %%T\Client2_VehicleNetDebug.txt

        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client2.log" > "%BASE_DIR%\%%T\Client2_VehicleNetAll.txt"
        echo Wrote %%T\Client2_VehicleNetAll.txt
    ) else (
        echo Missing %%T\Client2.log
    )
)

echo.
echo [CarFight] Done.
echo [CarFight] Send Server_VehicleNetDebug.txt, Server_VehicleNetAll.txt, *_VehicleNetDebug.txt, and *_VehicleNetAll.txt files for analysis.
pause
