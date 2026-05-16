@echo off
:: =============================================
:: Launch script for the editor and export
:: Usage:
::   run.bat          -> Launches the editor (./editor/main.py)
::   run.bat export   -> Exports .py files in editor/ to export.txt
::   run.bat /?       -> Displays this help
:: =============================================

:: Check arguments
if "%~1"=="" (
    :: Default mode: launch the editor
    python ./editor/main.py
    exit /b
) else if "%~1"=="export" (
    call :export_py_files
    exit /b
) else if "%~1"=="/?" (
    call :show_help
    exit /b
) else (
    echo Error: Unrecognized parameter "%~1".
    echo.
    call :show_help
    exit /b 1
)

:: =============================================
:: Function: Export .py files with metadata
:: =============================================
:export_py_files
setlocal enabledelayedexpansion
set "output=export.txt"
set "python_version="

:: Get Python version
for /f "tokens=2" %%v in ('python --version 2^>nul') do set "python_version=%%v"

:: Initialize output file
if exist "%output%" del /q "%output%"
echo Python Version: %python_version% > "%output%"
echo. >> "%output%"
echo ======================================= >> "%output%"
echo. >> "%output%"

:: Iterate over .py files in editor/
for /r ./editor %%F in (*.py) do (
    echo === [%~nF] (%~pF) === >> "%output%"
    echo. >> "%output%"
    type "%%F" >> "%output%"
    echo. >> "%output%"
    echo -------------------------------------- >> "%output%"
    echo. >> "%output%"
)

echo Files exported to %output%
endlocal
exit /b

:: =============================================
:: Function: Display help
:: =============================================
:show_help
echo Usage:
echo   run.bat          -> Launches the editor (./editor/main.py)
echo   run.bat export   -> Exports .py files in editor/ to export.txt
echo   run.bat /?       -> Displays this help
exit /b