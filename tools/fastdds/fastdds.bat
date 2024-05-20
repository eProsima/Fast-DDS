@echo off
setlocal

set dir=%~dp0
set args=%1

:getarg
shift
if "%~1"=="" goto continue
set args=%args% %1
goto getarg

:continue

:: Check python
py --list > NUL 2>&1

if not %ERRORLEVEL%==0 (
      echo python interpreter not found. Please, make sure its location is in the PATH environment variable.
      exit /B 65
   )
)

:: Python version in the form "Major.Minor"
if not "%PYTHON_VERSION%" == "" (
      :: Use launcher to profit from shebang hints on fastdds.py
      :: Select the correct python version to source the appropriate paths
      py -%PYTHON_VERSION% "%dir%\..\tools\fastdds\fastdds.py" %args%
) else (
      :: Use launcher to profit from shebang hints on fastdds.py
      :: Select latest available python version
      py "%dir%\..\tools\fastdds\fastdds.py" %args%
)

