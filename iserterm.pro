#include(./qtserialport/src/serialport/serialport-lib.pri)
#INCLUDEPATH += ./qtserialport/include

QT += widgets serialport

TARGET = iserterm
TEMPLATE = app
#GIT_VERSION = git describe --abbrev=4 --dirty --always --tags
GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

SOURCES += \
    multivalidator.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    consolesettingsdialog.cpp \
    serialthread.cpp \
    multistring.cpp \
    shiftdeleventfilter.cpp

HEADERS += \
    common.h \
    console.h \
    multivalidator.h \
    mainwindow.h \
    settingsdialog.h \
    version.h \
    consolesettingsdialog.h \
    serialthread.h \
    multistring.h \
    shiftdeleventfilter.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    consolesettingsdialog.ui

RESOURCES += \
    iserterm.qrc

RC_FILE = resources.rc
