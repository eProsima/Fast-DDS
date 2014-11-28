#-------------------------------------------------
#
# Project created by QtCreator 2014-06-26T09:08:43
#
#-------------------------------------------------

RTPSVERSION = 1.0.0

QT  += core
#QT  -= gui
QT += gui



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


unix:QMAKE_CXXFLAGS_DEBUG += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -g -std=c++0x -D__DEBUG
unix:QMAKE_CXXFLAGS_RELEASE += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -O2 -std=c++0x

win32:QMAKE_CXXFLAGS_DEBUG += -D_MBCS -D__DEBUG -DBOOST_ALL_DYN_LINK -D__LITTLE_ENDIAN__ -D_WIN32
win32:QMAKE_CXXFLAGS_RELEASE += -D_MBCS -DBOOST_ALL_DYN_LINK -D__LITTLE_ENDIAN__ -D_WIN32

win32:QMAKE_LFLAGS_WINDOWS +=/FORCE:MULTIPLE
win32:QMAKE_LFLAGS_WINDOWS_DLL +=/FORCE:MULTIPLE


#CONFIG += console


unix:CONFIG(debug, debug|release): TARGET = $$_PRO_FILE_PWD_/bin/ShapesDemod
unix:CONFIG(release, debug|release):TARGET = $$_PRO_FILE_PWD_/bin/ShapesDemo

win32:CONFIG(debug, debug|release): TARGET = ShapesDemod
win32:CONFIG(release, debug|release):TARGET = ShapesDemo


TEMPLATE = app

INCLUDEPATH += include/
               forms/ui/

FORMS    +=   forms/mainwindow.ui \
              forms/publishdialog.ui \
              forms/subscribedialog.ui \
              forms/optionsdialog.ui


UI_DIR = $$_PRO_FILE_PWD_/forms/ui/
MOC_DIR = $$_PRO_FILE_PWD_/forms/ui/

unix: CONFIG(release, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -lfastrtps
else:unix: CONFIG(debug, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -lfastrtpsd

INCLUDEPATH += $$PWD/../../include
INCLUDEPATH += $$PWD/../../thirdparty/eprosima-common-code
#DEPENDPATH += $$PWD/../../include
#DEPENDPATH += $$PWD/../../thirdparty/eprosima-common-code

unix: CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libfastrtpsd.a
else:unix: CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libfastrtps.a



unix: LIBS += -lboost_thread -lboost_system -lboost_date_time
#win32:INCLUDEPATH += $(LIB_BOOST_PATH)/
win32:LIBS += -L$(LIB_BOOST_PATH)/lib/i86Win32VS2010/


HEADERS += \
    include/eprosimashapesdemo/qt/DrawArea.h \
    include/eprosimashapesdemo/qt/mainwindow.h \
    include/eprosimashapesdemo/qt/publishdialog.h \
    include/eprosimashapesdemo/qt/subscribedialog.h \
    include/eprosimashapesdemo/shapesdemo/Shape.h \
    include/eprosimashapesdemo/shapesdemo/ShapesDemo.h \
    include/eprosimashapesdemo/shapesdemo/ShapeTopicDataType.h \
    include/eprosimashapesdemo/utils/md5.h \
    include/eprosimashapesdemo/shapesdemo/ShapePublisher.h \
    include/eprosimashapesdemo/qt/UpdateThread.h \
    include/eprosimashapesdemo/shapesdemo/ShapeSubscriber.h \
    include/eprosimashapesdemo/qt/optionsdialog.h \
    include/eprosimashapesdemo/qt/ContentFilterSelector.h \
    include/eprosimashapesdemo/shapesdemo/ShapeDefinitions.h \
    include/eprosimashapesdemo/shapesdemo/ShapeHistory.h


SOURCES += \
    src/qt/DrawArea.cpp \
    src/qt/mainwindow.cpp \
    src/qt/publishdialog.cpp \
    src/qt/subscribedialog.cpp \
    src/shapesdemo/ShapesDemo.cpp \
    src/shapesdemo/ShapeTopicDataType.cpp \
    src/utils/md5.cpp \
    src/main.cpp \
    src/shapesdemo/ShapePublisher.cpp \
    src/qt/UpdateThread.cpp \
    src/shapesdemo/ShapeSubscriber.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/ContentFilterSelector.cpp \
    src/shapesdemo/ShapeHistory.cpp




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/i86Win32VS2010 -llibfastrtps-1.0.0 -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lShlwapi -lIphlpapi
win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/i86Win32VS2010 -llibfastrtpsd-1.0.0 -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lShlwapi -lIphlpapi




#win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libfastrtps-1.0.0.lib
#win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libfastrtpsd-1.0.0.lib



#win32:INCLUDEPATH += $$PWD/../../include
#win32:DEPENDPATH += $$PWD/../../lib/i86Win32VS2010 C:\Windows\System32


#win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libfastrtps-0.5.1.lib Shlwapi.lib Iphlpapi.lib
#win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libfastrtpsd-0.5.1.lib Shlwapi.lib Iphlpapi.lib

RESOURCES += \
    images/eprosimalogo.qrc

RC_FILE = images/eprosima_icon.rc
