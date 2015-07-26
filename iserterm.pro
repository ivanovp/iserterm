#include(./qtserialport/src/serialport/serialport-lib.pri)
#INCLUDEPATH += ./qtserialport/include

QT += widgets serialport

TARGET = iserterm
TEMPLATE = app

SOURCES += \
    hexvalidator.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    consolesettingsdialog.cpp \
    serialthread.cpp

HEADERS += \
    common.h \
    console.h \
    hexvalidator.h \
    mainwindow.h \
    settingsdialog.h \
    version.h \
    consolesettingsdialog.h \
    serialthread.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    consolesettingsdialog.ui

RESOURCES += \
    iserterm.qrc

RC_FILE = resources.rc
