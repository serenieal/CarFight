@echo off
REM Vehicle WheelClass default synchronization script.
REM Version: v1.0.0
REM Changelog:
REM - v1.0.0: Synchronize BP_Wheel_Front/BP_Wheel_Rear mismatched defaults with DA_PoliceCar intent.
REM Migration:
REM - This script changes only BP_Wheel_Front and BP_Wheel_Rear saved defaults.
REM - It does not change vehicle input, RepMove, RepPhysics, steering limit, or camera settings.

REM Shared CarFight Unreal Engine environment guard.
call "%~dp0CarFightEnv.bat"
if errorlevel 1 (
	echo.
	pause
	exit /b %errorlevel%
)

echo [CarFight] Synchronizing BP wheel class defaults with DA_PoliceCar.
echo [CarFight] Engine: %CARFIGHT_EDITOR_CMD_EXE%
echo.

"%CARFIGHT_EDITOR_CMD_EXE%" "%CARFIGHT_UPROJECT%" -run=pythonscript -script="%~dp0SetVehicleWheelDefaults.py" -unattended -nop4 -nosplash -nullrhi
exit /b %errorlevel%
