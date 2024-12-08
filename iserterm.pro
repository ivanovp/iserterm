#include(./qtserialport/src/serialport/serialport-lib.pri)
#INCLUDEPATH += ./qtserialport/include

QT += widgets serialport

TARGET = iserterm
TEMPLATE = app
#GIT_VERSION = git describe --abbrev=4 --dirty --always --tags
GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

SOURCES += \
    src/multivalidator.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/settingsdialog.cpp \
    src/console.cpp \
    src/consolesettingsdialog.cpp \
    src/serialthread.cpp \
    src/serialsettings.cpp \
    src/multistring.cpp \
    src/shiftdeleventfilter.cpp \
    src/finddialog.cpp

HEADERS += \
    src/common.h \
    src/console.h \
    src/multivalidator.h \
    src/mainwindow.h \
    src/settingsdialog.h \
    src/version.h \
    src/consolesettingsdialog.h \
    src/serialthread.h \
    src/serialsettings.h \
    src/multistring.h \
    src/shiftdeleventfilter.h \
    src/finddialog.h

FORMS += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui \
    ui/consolesettingsdialog.ui \
    ui/finddialog.ui

RESOURCES += \
    iserterm.qrc

RC_FILE = resources.rc
