@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM CarFight editor build script.
REM Version: v1.3.0
REM Changelog:
REM - v1.3.0: Run the TargetSelect contract test once when RunTargetSelectTests.once.txt exists.
REM - v1.2.0: Add non-interactive execution for GoPyMCP while preserving manual pause behavior.
REM - v1.1.0: Keep the console open after a failed editor build and preserve its exit code.
REM - v1.0.0: Build CarFight_ReEditor only through the shared engine guard.

REM Non-interactive mode skips pause so automation can receive the original exit code.
set "CARFIGHT_NON_INTERACTIVE=0"
if /I "%~1"=="--non-interactive" set "CARFIGHT_NON_INTERACTIVE=1"

REM Shared CarFight Unreal Engine environment guard.
call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	set "BUILD_EXIT_CODE=!errorlevel!"
	echo.
	if "!CARFIGHT_NON_INTERACTIVE!"=="0" pause
	exit /b !BUILD_EXIT_CODE!
)

echo [CarFight] Building CarFight_ReEditor with:
echo %CARFIGHT_ENGINE_ROOT%
echo.

call "%CARFIGHT_BUILD_BAT%" CarFight_ReEditor Win64 Development -Project="%CARFIGHT_UPROJECT%" -WaitMutex -NoHotReloadFromIDE
set "BUILD_EXIT_CODE=!errorlevel!"

REM Optional one-shot TargetSelect contract test hook. The marker is removed before execution.
if "!BUILD_EXIT_CODE!"=="0" if exist "%~dp0RunTargetSelectTests.once.txt" (
	del /q "%~dp0RunTargetSelectTests.once.txt"
	call "%~dp0RunTargetSelectTests.bat" --non-interactive
	set "BUILD_EXIT_CODE=!errorlevel!"
)

if not "!BUILD_EXIT_CODE!"=="0" (
	echo.
	echo [CarFight] Editor build failed. Exit code: !BUILD_EXIT_CODE!
	if "!CARFIGHT_NON_INTERACTIVE!"=="0" (
		echo [CarFight] Press any key to close this window.
		pause >nul
	)
)

exit /b !BUILD_EXIT_CODE!
