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
:: call build_cdr.bat
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd "..\..\..\..\"

:: Get the current vesion of RTPS
call %EPROSIMADIR%\scripts\common_pack_functions.bat :getVersionFromCPP VERSIONRTPS include/eprosimartps/eprosimartps_version.h
if not %errorstatus%==0 goto :exit

:: Compile RTPS for target.
cd "utils\scripts"
:: call build_ertps.bat
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
cd "..\.."

:: Create PDFS from documentation.
cd "doc"
mkdir pdf
mkdir html
:: Installation manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\RTPS - Installation Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
:: User manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\RTPS - User Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit
:: RTPSGEN User manual
soffice.exe --headless "macro:///eProsima.documentation.changeVersion(%CD%\RTPSGEN - User Manual.odt,%VERSIONRTPS%)"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

:: Copy pfd files into pdf dir
copy "RTPS - Installation Manual.pdf" "pdf\RTPS - Installation Manual.pdf"
del "RTPS - Installation Manual.pdf"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

copy "RTPS - User Manual.pdf" "pdf\RTPS - User Manual.pdf"
del "RTPS - User Manual.pdf"
set errorstatus=%ERRORLEVEL%
if not %errorstatus%==0 goto :exit

copy "RTPSGEN - User Manual.pdf" "pdf\RTPSGEN - User Manual.pdf"
del "RTPSGEN - User Manual.pdf"
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
ren refman.pdf "RTPS - API C++ Manual.pdf"
copy "RTPS - API C++ Manual.pdf" "..\..\..\..\..\doc\pdf\RTPS - API C++ Manual.pdf"
cd "..\..\..\..\.."

:: Build utilities
cd utils/ShapesDemo
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
call qmake ShapesDemo.pro -r -spec win32-msvc2010
call nmake clean
call nmake
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
cd ..\..

cd rtpsgen
call ant jars
cd ..

:: Create installers.
cd "utils\installers\ertps\windows"
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
