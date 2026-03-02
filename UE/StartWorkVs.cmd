@echo off
setlocal EnableExtensions

REM ===========================================================
REM StartWorkVs.cmd (launcher)
REM v2.0.0
REM
REM NOTE:
REM - Keep this launcher ASCII / no-BOM.
REM - Main logic is in StartWorkVs_v2_0_0.py (UTF-8 friendly).
REM - Avoids PowerShell ExecutionPolicy issues.
REM ===========================================================

REM [VAR] Python launcher executable
set "PY_LAUNCHER=py"

REM [VAR] Python fallback executable
set "PY_FALLBACK=python"

REM [VAR] Script path (same folder as this cmd)
set "PY_SCRIPT=%~dp0StartWorkVs.py"

if not exist "%PY_SCRIPT%" (
  echo [ERROR] Python script not found: "%PY_SCRIPT%"
  pause
  exit /b 1
)

where /q %PY_LAUNCHER%
if not errorlevel 1 (
  %PY_LAUNCHER% -3 "%PY_SCRIPT%"
) else (
  %PY_FALLBACK% "%PY_SCRIPT%"
)

set "EXITCODE=%errorlevel%"

echo.
echo [EXIT] code=%EXITCODE%
pause
exit /b %EXITCODE%
