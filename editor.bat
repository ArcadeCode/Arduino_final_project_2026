@echo off
:: =============================================
:: Launch script for the editor and export
:: Usage:
::   editor.bat          - Launches the editor
::   editor.bat export   - Exports .py files
::   editor.bat /?       - Displays help
:: =============================================

:: Check arguments
if "%~1"=="" (
    python "./editor/main.py"
    exit /b
)

if /i "%~1"=="export" (
    goto :export_py_files
)

if "%~1"=="/?" (
    goto :show_help
)

echo Error: Unrecognized parameter "%~1".
echo.
goto :show_help


:: =============================================
:: Function: Export .py files with metadata
:: =============================================
:export_py_files
setlocal EnableDelayedExpansion

set "output=export.txt"

:: Get Python version
for /f "tokens=2" %%v in ('python --version 2^>^&1') do (
    set "python_version=%%v"
)

:: Initialize output file
(
    echo Python Version: !python_version!
    echo.
    echo =======================================
    echo.
) > "%output%"

:: Export each .py file
for /r "./editor" %%F in (*.py) do (

    echo Exporting: %%F

    (
        echo === [%%~nF] [%%~dpF] ===
        echo.
        type "%%F"
        echo.
        echo --------------------------------------
        echo.
    ) >> "%output%"
)

echo.
echo Files exported to "%output%"

endlocal
exit /b


:: =============================================
:: Function: Display help
:: =============================================
:show_help
echo Usage:
echo   editor.bat          - Launches the editor
echo   editor.bat export   - Exports .py files to export.txt
echo   editor.bat /?       - Displays help
exit /b