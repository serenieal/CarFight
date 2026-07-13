@echo off
REM CarFight editor build script.
REM Version: v1.1.0
REM Changelog:
REM - v1.1.0: Keep the console open after a failed editor build and preserve its exit code.
REM - v1.0.0: Build CarFight_ReEditor only through the shared engine guard.

REM Shared CarFight Unreal Engine environment guard.
call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	echo.
	pause
	exit /b %errorlevel%
)

echo [CarFight] Building CarFight_ReEditor with:
echo %CARFIGHT_ENGINE_ROOT%
echo.

call "%CARFIGHT_BUILD_BAT%" CarFight_ReEditor Win64 Development -Project="%CARFIGHT_UPROJECT%" -WaitMutex -NoHotReloadFromIDE
set "BUILD_EXIT_CODE=%errorlevel%"

if not "%BUILD_EXIT_CODE%"=="0" (
	echo.
	echo [CarFight] Editor build failed. Exit code: %BUILD_EXIT_CODE%
	echo [CarFight] Press any key to close this window.
	pause >nul
)

exit /b %BUILD_EXIT_CODE%
