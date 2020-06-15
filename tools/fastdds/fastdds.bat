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
python --version > NUL 2>&1

if not %ERRORLEVEL%==0 (
      echo python interpreter not found. Please, make sure its location is in the PATH environment variable.
      exit /B 65
   )
)

python "%dir%\..\tools\fastdds\fastdds.py" %args%