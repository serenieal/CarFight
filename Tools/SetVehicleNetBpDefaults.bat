@echo off
REM Vehicle BP network default cleanup script.
REM Version: v1.6.0
REM Changelog:
REM - v1.6.0: Validate BP compile/save after deleting failed vehicle network experiment UPROPERTYs.
REM - v1.5.0: Treat hidden experimental UPROPERTY defaults as normal skips after editor exposure cleanup.
REM - v1.4.0: Disable AuthorityGuard by default because fall prevention now belongs to map boundaries.
REM - v1.3.0: Reset failed vehicle network experiments to the clean baseline.
REM - v1.2.0: Reset Legacy input RPC baseline defaults and safer authority fall recovery defaults.
REM - v1.1.0: Reset engine physics baseline network defaults.
REM - v1.0.0: Run Unreal Python script that resets legacy RemoteInterp BP defaults.

REM Shared CarFight Unreal Engine environment guard.
call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	echo.
	pause
	exit /b %errorlevel%
)

echo [CarFight] Validating BP_CFVehiclePawn after vehicle network source cleanup.
echo [CarFight] Engine: %CARFIGHT_EDITOR_CMD_EXE%
echo.

"%CARFIGHT_EDITOR_CMD_EXE%" "%CARFIGHT_UPROJECT%" -run=pythonscript -script="%~dp0SetVehicleNetBpDefaults.py" -unattended -nop4 -nosplash -nullrhi
exit /b %errorlevel%
