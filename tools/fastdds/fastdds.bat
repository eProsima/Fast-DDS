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

if %PYTHON_VERSION%=="" (
      echo error retrieving python version. Please, make sure python is installed and accessible.
      exit /B 65
   )
)

:: Use launcher to profit from shebang hints on fastdds.py
py -%PYTHON_VERSION% "%dir%\..\tools\fastdds\fastdds.py" %args%
