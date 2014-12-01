::Build ShapesDemo application

set LIB_BOOST_PATH=C:\local\boost_1_57_0
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
call qmake ShapesDemo.pro -r -spec win32-msvc2010
call nmake clean
call nmake
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
::Copy necessary auxiliary files
mkdir files\bin
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_date_time-vc100-mt-1_57.dll" "files\bin\boost_date_time-vc100-mt-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_date_time-vc100-mt-gd-1_57.dll" "files\bin\boost_date_time-vc100-mt-gd-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_signals-vc100-mt-1_57.dll" "files\bin\boost_signals-vc100-mt-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_signals-vc100-mt-gd-1_57.dll" "files\bin\boost_signals-vc100-mt-gd-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_system-vc100-mt-1_57.dll" "files\bin\boost_system-vc100-mt-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_system-vc100-mt-gd-1_57.dll" "files\bin\boost_system-vc100-mt-gd-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_thread-vc100-mt-1_57.dll" "files\bin\boost_thread-vc100-mt-1_53.dll"
copy "%LIB_BOOST_PATH%\lib\i86Win32VS2010\boost_thread-vc100-mt-gd-1_57.dll" "files\bin\boost_thread-vc100-mt-gd-1_53.dll"
mkdir files\bin\platforms
set QTDIR=C:\Qt\5.3\msvc2010_opengl
copy "%QTDIR%\plugins\platforms\qminimal.dll" "files\bin\platforms\qminimal.dll"
copy "%QTDIR%\plugins\platforms\qoffscreen.dll" "files\bin\platforms\qoffscreen.dll"
copy "%QTDIR%\plugins\platforms\qwindows.dll" "files\bin\platforms\qwindows.dll"
copy "%QTDIR%\bin\Qt5Core.dll" "files\bin\Qt5Core.dll"
copy "%QTDIR%\bin\Qt5Gui.dll" "files\bin\Qt5Gui.dll"
copy "%QTDIR%\bin\Qt5Widgets.dll" "files\bin\Qt5Widgets.dll"
copy "%QTDIR%\bin\icudt52.dll" "files\bin\icudt52.dll"
copy "%QTDIR%\bin\icuin52.dll" "files\bin\icuin52.dll"
copy "%QTDIR%\bin\icuuc52.dll" "files\bin\icuuc52.dll"
copy "C:\Windows\System32\shlwapi.dll" "files\bin\shlwapi.dll"
copy "C:\Windows\System32\iphlpapi.dll" "files\bin\iphlpapi.dll"