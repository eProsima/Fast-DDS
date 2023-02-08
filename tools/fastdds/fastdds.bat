@echo off

rem Check python
set _interpreter=
python --version | findstr /C:"Python 3" >nul
if %ERRORLEVEL% equ 0 set _interpreter=python
if %ERRORLEVEL% equ 1 py -0 2>&1 | findstr 3. >nul && set _interpreter=py -3

if not defined _interpreter (
  echo Suitable python interpreter not found.
  echo Please, make sure its location is in the PATH environment variable.
  exit /B 42 
)

rem Use launcher to profit from shebang hints on fastdds.py
%_interpreter% %~dp0..\tools\fastdds\fastdds.py %*
