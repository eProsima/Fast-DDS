:: This script build CDR library for any platform.

@echo off

:: Initialize the returned value to 0 (all succesfully)
set errorstatus=0

:: Get the current vesion of CDR
call thirdparty\dev-env\scripts\common_pack_functions.bat :getVersionFromCPP VERSIONRTPS ..\..\include\fastrtps\fastrtps_version.h
if not %errorstatus%==0 goto :exit

set WIN32VER=%1
set MSBUILDEXE=%2
:: i86 Platform

:: Release DLL Configuration
:: Clean the visual solution
if "%3"=="clean"   %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="ReleaseDLL" /p:Platform="%WIN32VER%" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="ReleaseDLL" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Debug DLL Configuration
:: Clean the visual solution
if "%3"=="clean"   %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="DebugDLL" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="DebugDLL" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Release Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="Release" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="Release" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Debug Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="Debug" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="Debug" /p:Platform="Win32" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: x64 Platform

:: Release DLL Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="ReleaseDLL" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="ReleaseDLL" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Debug DLL Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="DebugDLL" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="DebugDLL" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Release Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="Release" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="Release" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Debug Configuration
:: Clean the visual solution
if "%3"=="clean" %MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Clean /p:Configuration="Debug" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
:: Build the visual solution
%MSBUILDEXE% "..\..\%WIN32VER%\cpp\eRTPS\eRTPS.sln" /t:Build /p:Configuration="Debug" /p:Platform="x64" /p:VERSION="-%VERSIONRTPS%"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

goto :exit

:: Function exit ::
:exit
if %errorstatus%==0 (echo "BUILD SUCCESSFULLY") else (echo "BUILD FAILED")
goto :EOF

