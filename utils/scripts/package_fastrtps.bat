:: This script packs DynamicFastBuffers library for any platform in Windows.
::
:: This script needs the next environment variables to be run.
:: - SVN_BIN_DIR: Directory with the subversion binaries.
:: - LIBREOFFICE_BIN_DIR: Directory with the LibreOffice binaries.
:: - NSIS_BIN_DIR: Directory with the NSIS installer libraries.
:: - EPROSIMADIR: URL to the directory with common sources of eProsima.
:: - ANT_BIN_DIR: Directory with the ant binaries.
:: - DOXYGEN_BIN_DIR: Directory with the doxygen binaries. (Also pdflatex and graphviz)
:: Also this script needs the eProsima.documentation.changeVersion macro installed in the system.

setlocal EnableDelayedExpansion
setlocal EnableExpansion
@echo off

:: Initialize the returned value to 0 (all succesfully)
set errorstatus=0

:: Go to root directory
cd "..\.."



:: Compile CDR library.
cd "thirdparty\fastcdr"
cd "utils\scripts"
call build_cdr.bat noclean
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd "..\..\..\..\"

:: Get the current vesion of RTPS
call %EPROSIMADIR%\scripts\common_pack_functions.bat :getVersionFromCPP VERSIONRTPS include/fastrtps/fastrtps_version.h
if not %errorstatus%==0 goto :exit

:: Compile RTPS for target.
cd "utils\scripts"
::COMPILE VS2013
set LIB_BOOST_PATH=C:\local\boost_1_57_0
call build_fastrtps.bat noclean
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd "..\.."

:: Create PDFS from documentation.
cd "doc"
mkdir pdf
mkdir html
:: Installation manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\FastRTPS_Installation_Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
:: User manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\FastRTPS_User_Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
:: RTPSGEN User manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\FASTRTPSGEN_User_Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Copy pfd files into pdf dir
copy "FastRTPS_Installation_Manual.pdf" "pdf\FastRTPS_Installation_Manual.pdf"
del "FastRTPS_Installation_Manual.pdf"
set errorstatus=%ERRORLEVEL% 
if not %errorstatus%==0 goto :exit

copy "FastRTPS_User_Manual.pdf" "pdf\FastRTPS_User_Manual.pdf"
del "FastRTPS_User_Manual.pdf"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

copy "FASTRTPSGEN_User_Manual.pdf" "pdf\FASTRTPSGEN_User_Manual.pdf"
del "FASTRTPSGEN_User_Manual.pdf"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

cd ".."
:: Create README
soffice.exe --headless "macro:///eProsima.documentation.changeVersionToHTML(%CD%\README.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Create doxygen information.
:: Generate the examples
:: Export version
set VERSION_DOX=%VERSIONRTPS%
mkdir utils\doxygen\output
mkdir utils\doxygen\output\doxygen
mkdir utils\doxygen\output\doxygen\html
mkdir utils\doxygen\output\doxygen\latex
cd "utils\doxygen"

doxygen doxyfile_public_api

set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd output\doxygen\latex

call make.bat

set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
ren refman.pdf "FastRTPS_API_C++_Manual.pdf"
copy "FastRTPS_API_C++_Manual.pdf" "..\..\..\..\..\doc\pdf\FastRTPS_API_C++_Manual.pdf"
cd "..\..\..\..\.."

:: Build utilities
cd utils/ShapesDemo
call build_shapesdemo.bat
cd ..\..

cd rtpsgen
call ant jars
cd ..

:: Create installers.
cd "utils\installers\fastrtps\windows"
:: Win32 installer.
makensis.exe /DVERSION="%VERSIONRTPS%" setup.nsi
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd "..\..\..\.."
:: rd /S /Q "utils\doxygen\output"


rmdir /S /Q utils\doxygen\output
rmdir /S /Q doc\html
rmdir /S /Q doc\pdf

goto :exit

:: Function exit ::
:exit
if %errorstatus%==0 (echo "PACKAGING SUCCESSFULLY") else (echo "PACKAGING FAILED")
exit /b %errorstatus%
goto :EOF
