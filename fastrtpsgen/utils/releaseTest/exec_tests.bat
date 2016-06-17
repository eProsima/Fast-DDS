:: Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

:: This script runs all tests in this directory and checks their results. Supports two optional parameters.

setlocal EnableDelayedExpansion
@echo off


echo "STARTING TESTS"
:: Initialize the returned value to 0 (all succesfully)
set errorstatus=0
set globalerrorstatus=0
set configuration=x64Win64VS2013
set platform=x64
set failedtests=
set correcttests=
set configurations=Release DLL,Release,Debug DLL,Debug

::for %%a in ("%configurations:,=" "%") do (
::	echo "CONFIGURATION %%a"
::)


:: Get number of arguments
set argC=0
for %%x in (%*) do set /A argC+=1
echo "Provided number of arguments: %argC%"
:: Get the optional parameter
if "%argC%" == "1" (
	echo "EXECUTING TEST %1"
	cd %1
	call :execTest
	cd ..
	if !errorstatus!==0 	(
		echo TEST %1 SUCESSFUL
	) else 	(
		echo TEST %1 FAILED
		set globalerrorstatus=1
		set failedtests=%1
	)
) else (
	echo "EXECUTING ALL TESTS IN THIS DIRECTORY"
	for /D %%D in ("*") do 	(
		cd %%D
		call :execTest
		cd ..
		if !errorstatus!==0 (
			echo "TEST %%D SUCESSFUL"
			set "correcttests=!correcttests! %%~NXD"
		) else (
			echo "TEST %%D FAILED"
			set globalerrorstatus=1
			set "failedtests=!failedtests! %%~NXD"
		)
	)
)
echo "ALL TESTS HAVE FINISHED, RESULTS:"
if !globalerrorstatus!==0 (
    echo "ALL TESTS SUCCESSFULL"
) else (
    echo "TESTS FAILED: %failedtests%"
	echo "TESTS OK: %correcttests%"
)

exit /b %globalerrorstatus%

:: This function execute a test in a directory. Use "call" to call this method.
:: @param Target used.
:: @param Plaform for Visual Studio.
:: @param Name of the test
:execTest
set errorstatus=0
::Get the IDL files:
echo "EXECUTING TEST"
set "IDLFILES= "
for %%i in (*.idl) do (
	::echo %%i
	set "IDLFILES=!IDLFILES! %%~NXi"
)
echo "FOUND IDL FILES: %IDLFILES%"
::Generate Info
copy ..\..\..\lib\fastrtpsgen.jar .
java -jar fastrtpsgen.jar -local -example %configuration% -replace %IDLFILES%
if errorlevel 1 (
	set errorstatus=1
	goto :EOF
)
if exist *.sln ( 
	echo "Solution exists, compiling"
	for %%s in ("%configurations:,=" "%") do (
		echo "****************Compiling Configuration %%s **********************"
		:: Clean the visual solution
		msbuild "solution-%configuration%.sln" /t:Clean /p:Configuration=%%s /p:Platform="%platform%"
		:: Build the visual solution
		msbuild "solution-%configuration%.sln" /t:Build /p:Configuration=%%s /p:Platform="%platform%"
		if errorlevel 1 (
			set errorstatus=1
			goto :EOF
		)
	)
	rd /s /q x64
	rd /s /q bin
	rd /s /q lib
	del *.sln
	del *.vcxproj
	del *.filters
)
del *.h
del *.cxx
del *.jar

goto :EOF
