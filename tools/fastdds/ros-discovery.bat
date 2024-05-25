@echo off
setlocal

set dir=%~dp0

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
      py -%PYTHON_VERSION% "%dir%\..\tools\fastdds\fastdds.py" discovery %*
) else (
	  :: Use launcher to profit from shebang hints on fastdds.py
      :: Select latest available python version
      py "%dir%\..\tools\fastdds\fastdds.py" discovery %*
)

