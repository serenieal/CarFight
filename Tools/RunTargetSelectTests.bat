@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM CarFight TargetSelect automation test runner.
REM Version: v1.1.0
REM Changelog:
REM - v1.1.0: Run the complete CarFight.TargetSelect automation suite including TS-P0-01 and TS-P0-02.
REM - v1.0.0: Run the CarFight.TargetSelect.TS_P0_01.RuntimeContract editor automation test.

set "CARFIGHT_NON_INTERACTIVE=0"
if /I "%~1"=="--non-interactive" set "CARFIGHT_NON_INTERACTIVE=1"

call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	set "TEST_EXIT_CODE=!errorlevel!"
	if "!CARFIGHT_NON_INTERACTIVE!"=="0" pause
	exit /b !TEST_EXIT_CODE!
)

set "TARGET_SELECT_REPORT_DIR=%CARFIGHT_UE_DIR%\Saved\Automation\TargetSelect"
if exist "%TARGET_SELECT_REPORT_DIR%" rmdir /s /q "%TARGET_SELECT_REPORT_DIR%"

echo [CarFight] Running complete TargetSelect automation suite...
echo [CarFight] Report: %TARGET_SELECT_REPORT_DIR%
echo.

"%CARFIGHT_EDITOR_CMD_EXE%" "%CARFIGHT_UPROJECT%" -unattended -nop4 -NullRHI -NoSound -NoSplash -stdout -FullStdOutLogOutput -ExecCmds="Automation RunTests CarFight.TargetSelect; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath="%TARGET_SELECT_REPORT_DIR%"
set "TEST_EXIT_CODE=!errorlevel!"

if not "!TEST_EXIT_CODE!"=="0" (
	echo.
	echo [CarFight] TargetSelect automation suite failed. Exit code: !TEST_EXIT_CODE!
	if "!CARFIGHT_NON_INTERACTIVE!"=="0" pause
	exit /b !TEST_EXIT_CODE!
)

echo.
echo [CarFight] TargetSelect automation suite passed.
if "!CARFIGHT_NON_INTERACTIVE!"=="0" pause
exit /b 0