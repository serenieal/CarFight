@echo off
REM CarFight editor launch script.
REM Version: v1.0.0
REM Changelog:
REM - v1.0.0: Launch the project editor only through the shared engine guard.

REM Shared CarFight Unreal Engine environment guard.
call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	echo.
	pause
	exit /b %errorlevel%
)

echo [CarFight] Launching editor with:
echo %CARFIGHT_EDITOR_EXE%
echo.

start "CarFight Editor" "%CARFIGHT_EDITOR_EXE%" "%CARFIGHT_UPROJECT%" -log
exit /b 0
