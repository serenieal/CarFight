@echo off
REM CarFight editor launch script.
REM Version: v1.1.0
REM Changelog:
REM - v1.1.0: Canonical Editor launch now always starts Unreal MCP and pins its listener to CarFight port 8100, removing dependence on per-user Auto Start settings.
REM - v1.0.0: Launch the project editor only through the shared engine guard.
REM Migration:
REM - EditorPerProjectUserSettings Auto Start may be on or off; canonical CarFight launch owns MCP startup through command-line flags.

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

REM Canonical CarFight Unreal MCP listener port consumed by GoPyMCP and .codex/config.toml.
set "CARFIGHT_MCP_PORT=8100"

REM Start Unreal MCP independently of volatile EditorPerProjectUserSettings Auto Start state.
start "CarFight Editor" "%CARFIGHT_EDITOR_EXE%" "%CARFIGHT_UPROJECT%" -log -ModelContextProtocolStartServer -ModelContextProtocolPort=%CARFIGHT_MCP_PORT%
exit /b 0
