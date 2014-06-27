#-------------------------------------------------
#
# Project created by QtCreator 2014-06-26T09:08:43
#
#-------------------------------------------------



QT  += core
QT  -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


QMAKE_CXXFLAGS_DEBUG += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -g -std=c++0x -D__DEBUG
QMAKE_CXXFLAGS += -c -Wall -D__LITTLE_ENDIAN__ -m64 -fpic -O2 -std=c++0x


CONFIG += console

CONFIG(debug, debug|release) {
    TARGET = $$_PRO_FILE_PWD_/bin/ShapesDemod
} else {
    TARGET = $$_PRO_FILE_PWD_/bin/ShapesDemo
}

TEMPLATE = app

INCLUDEPATH += include/
               forms/ui/

FORMS    +=   forms/mainwindow.ui \
                forms/publishdialog.ui \
    forms/subscribedialog.ui
UI_DIR = $$_PRO_FILE_PWD_/forms/ui/
MOC_DIR = $$_PRO_FILE_PWD_/forms/ui/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartps
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartpsd
else:unix: CONFIG(release, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartps
else:unix: CONFIG(debug, debug|release):LIBS += -L$$PWD/../../lib/x64Linux2.6gcc/ -leprosimartpsd


INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartps.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartpsd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/eprosimartps.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/eprosimartpsd.lib
else:unix: CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartpsd.a
else:unix: CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/x64Linux2.6gcc/libeprosimartps.a

unix:!macx|win32: LIBS += -lboost_thread -lboost_system

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
    include/eprosimashapesdemo/qt/QDebugStream.h

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
    src/main.cpp



