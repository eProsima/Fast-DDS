#-------------------------------------------------
#
# Project created by QtCreator 2014-06-26T09:08:43
#
#-------------------------------------------------

RTPSVERSION = 0.5.0

QT  += core
QT  -= gui
#QT += gui



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


unix:QMAKE_CXXFLAGS_DEBUG += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -g -std=c++0x -D__DEBUG
unix:QMAKE_CXXFLAGS_RELEASE += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -O2 -std=c++0x

win32:QMAKE_CXXFLAGS_DEBUG += -D_MBCS -D__DEBUG -DBOOST_ALL_DYN_LINK -D__LITTLE_ENDIAN__ -D_WIN32
win32:QMAKE_CXXFLAGS_RELEASE += -D_MBCS -DBOOST_ALL_DYN_LINK -D__LITTLE_ENDIAN__ -D_WIN32

win32:QMAKE_LFLAGS_WINDOWS +=/FORCE:MULTIPLE
win32:QMAKE_LFLAGS_WINDOWS_DLL +=/FORCE:MULTIPLE


CONFIG += console


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

unix: CONFIG(release, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartps
else:unix: CONFIG(debug, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartpsd

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

unix: CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartpsd.a
else:unix: CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartps.a


unix: LIBS += -lboost_thread -lboost_system
win32:INCLUDEPATH += C:/local/boost_1_53_0/
win32:LIBS += -LC:/local/boost_1_53_0/stage/lib/x86/

HEADERS += \
    include/eprosimashapesdemo/qt/DrawArea.h \
    include/eprosimashapesdemo/qt/mainwindow.h \
    include/eprosimashapesdemo/qt/publishdialog.h \
    include/eprosimashapesdemo/qt/subscribedialog.h \
    include/eprosimashapesdemo/shapesdemo/Shape.h \
    include/eprosimashapesdemo/shapesdemo/ShapesDemo.h \
    include/eprosimashapesdemo/shapesdemo/ShapeTopicDataType.h \
    include/eprosimashapesdemo/shapesdemo/ShapeType.h \
    include/eprosimashapesdemo/utils/md5.h \
    include/eprosimashapesdemo/shapesdemo/ShapePublisher.h \
    include/eprosimashapesdemo/qt/UpdateThread.h \
    include/eprosimashapesdemo/shapesdemo/ShapeSubscriber.h \
    include/eprosimashapesdemo/qt/optionsdialog.h \
    include/eprosimashapesdemo/qt/ContentFilterSelector.h


SOURCES += \
    src/qt/DrawArea.cpp \
    src/qt/mainwindow.cpp \
    src/qt/publishdialog.cpp \
    src/qt/subscribedialog.cpp \
    src/shapesdemo/Shape.cpp \
    src/shapesdemo/ShapesDemo.cpp \
    src/shapesdemo/ShapeTopicDataType.cpp \
    src/shapesdemo/ShapeType.cpp \
    src/utils/md5.cpp \
    src/main.cpp \
    src/shapesdemo/ShapePublisher.cpp \
    src/qt/UpdateThread.cpp \
    src/shapesdemo/ShapeSubscriber.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/ContentFilterSelector.cpp




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/i86Win32VS2010/ -llibeprosimartps-0.5.0 -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lShlwapi -lIphlpapi
win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/i86Win32VS2010/ -llibeprosimartpsd-0.5.0 -L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib" -lShlwapi -lIphlpapi


win32:INCLUDEPATH += $$PWD/../../include
win32:DEPENDPATH += $$PWD/../../lib/i86Win32VS2010 C:\Windows\System32


#win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libeprosimartps-0.5.0.lib Shlwapi.lib Iphlpapi.lib
#win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/i86Win32VS2010/libeprosimartpsd-0.5.0.lib Shlwapi.lib Iphlpapi.lib

RESOURCES += \
    images/eprosimalogo.qrc

