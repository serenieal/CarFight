@echo off
REM CarFight vehicle network log extraction script.
REM Version: v0.2.0
REM Changelog:
REM - v0.2.0: Extract VehicleNetStateBase and combined vehicle network logs.
REM - v0.1.0: Extract VehicleNetDebug logs.

set "ROOT_DIR=D:\Work\CarFight_git"
set "BASE_DIR=%ROOT_DIR%\RuntimeLogs\VehicleNet"

echo [CarFight] Extract vehicle network log lines.
echo.

for %%T in (A_RepMoveTrue B_RepMoveFalse) do (
    if exist "%BASE_DIR%\%%T\Client1.log" (
        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client1.log" > "%BASE_DIR%\%%T\Client1_VehicleNetDebug.txt"
        echo Wrote %%T\Client1_VehicleNetDebug.txt

        findstr /C:"VehicleNetStateBase" "%BASE_DIR%\%%T\Client1.log" > "%BASE_DIR%\%%T\Client1_VehicleNetStateBase.txt"
        echo Wrote %%T\Client1_VehicleNetStateBase.txt

        findstr /C:"VehicleNetDebug" /C:"VehicleNetStateBase" "%BASE_DIR%\%%T\Client1.log" > "%BASE_DIR%\%%T\Client1_VehicleNetAll.txt"
        echo Wrote %%T\Client1_VehicleNetAll.txt
    ) else (
        echo Missing %%T\Client1.log
    )

    if exist "%BASE_DIR%\%%T\Client2.log" (
        findstr /C:"VehicleNetDebug" "%BASE_DIR%\%%T\Client2.log" > "%BASE_DIR%\%%T\Client2_VehicleNetDebug.txt"
        echo Wrote %%T\Client2_VehicleNetDebug.txt

        findstr /C:"VehicleNetStateBase" "%BASE_DIR%\%%T\Client2.log" > "%BASE_DIR%\%%T\Client2_VehicleNetStateBase.txt"
        echo Wrote %%T\Client2_VehicleNetStateBase.txt

        findstr /C:"VehicleNetDebug" /C:"VehicleNetStateBase" "%BASE_DIR%\%%T\Client2.log" > "%BASE_DIR%\%%T\Client2_VehicleNetAll.txt"
        echo Wrote %%T\Client2_VehicleNetAll.txt
    ) else (
        echo Missing %%T\Client2.log
    )
)

echo.
echo [CarFight] Done.
echo [CarFight] Send *_VehicleNetDebug.txt, *_VehicleNetStateBase.txt, and *_VehicleNetAll.txt files for analysis.
pause
