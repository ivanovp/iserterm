/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2024 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include <QSettings>
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QDebug>

#include "serialsettings.h"
#include "common.h"

QT_USE_NAMESPACE

SerialSettings::SerialSettings(QObject *parent) :
    QObject(parent)
{
#if WINDOWS
    m_serialSettings.name = "COM1";
#else
    m_serialSettings.name = "/dev/ttyUSB0";
#endif
    m_serialSettings.baudRate = 115200;
    m_serialSettings.stringBaudRate = "115200";
    m_serialSettings.dataBits = QSerialPort::Data8;
    m_serialSettings.parity = QSerialPort::NoParity;
    m_serialSettings.stringParity = "None";
    m_serialSettings.stopBits = QSerialPort::OneStop;
    m_serialSettings.stringStopBits = "1";
    m_serialSettings.flowControl = QSerialPort::NoFlowControl;
    m_serialSettings.stringFlowControl = "No handshake";
}

SerialSettings::~SerialSettings()
{
}

void SerialSettings::loadSettings(SerialSettings::serialSettings_t *serialSettings, QString profileName)
{
    QSettings settings;
    QString path = "serial/";

    if (!profileName.isEmpty())
    {
        path = "profile/" + profileName + "/";
        qDebug() << __PRETTY_FUNCTION__ << "### profile" << profileName;
    }

    serialSettings->name = settings.value (path + "name").toString();
    serialSettings->baudRate = settings.value (path + "baudRate", m_serialSettings.baudRate).toInt();
    serialSettings->dataBits = static_cast<QSerialPort::DataBits> (settings.value (path + "dataBits", m_serialSettings.dataBits).toInt());
    serialSettings->parity = static_cast<QSerialPort::Parity> (settings.value (path + "parity", m_serialSettings.parity).toInt());
    serialSettings->stringParity = settings.value (path + "stringParity", m_serialSettings.stringParity).toString();
    serialSettings->stopBits = static_cast<QSerialPort::StopBits> (settings.value (path + "stopBits", m_serialSettings.stopBits).toInt());
    serialSettings->stringStopBits = settings.value (path + "stringStopBits", m_serialSettings.stringStopBits).toString();
    serialSettings->flowControl = static_cast<QSerialPort::FlowControl> (settings.value (path + "flowControl", m_serialSettings.flowControl).toInt());
    serialSettings->stringFlowControl = settings.value (path + "stringFlowControl", m_serialSettings.stringFlowControl).toString();
    qDebug() << __PRETTY_FUNCTION__ << "name" << serialSettings->name;
    qDebug() << __PRETTY_FUNCTION__ << "baudRate" << serialSettings->baudRate;
    qDebug() << __PRETTY_FUNCTION__ << "dataBits" << serialSettings->dataBits;
    qDebug() << __PRETTY_FUNCTION__ << "parity" << serialSettings->parity;
    qDebug() << __PRETTY_FUNCTION__ << "stringParity" << serialSettings->stringParity;
    qDebug() << __PRETTY_FUNCTION__ << "stopBits" << serialSettings->stopBits;
    qDebug() << __PRETTY_FUNCTION__ << "stringStopBits" << serialSettings->stringStopBits;
    qDebug() << __PRETTY_FUNCTION__ << "flowControl" << serialSettings->flowControl;
    qDebug() << __PRETTY_FUNCTION__ << "stringFlowControl" << serialSettings->stringFlowControl;
}

void SerialSettings::saveSettings(SerialSettings::serialSettings_t *serialSettings, QString profileName)
{
    QSettings settings;
    QString path = "serial/";

    if (!profileName.isEmpty())
    {
        /* Replace '/' to 0x7F, because '/' is the separator... */
        profileName.replace(SEP_CHAR, REPL_CHAR);
        path = "profile/" + profileName + "/";
        qDebug() << __PRETTY_FUNCTION__ << "### profile" << profileName;
    }

    settings.setValue (path + "name", serialSettings->name);
    settings.setValue (path + "baudRate", serialSettings->baudRate);
    settings.setValue (path + "dataBits", serialSettings->dataBits);
    settings.setValue (path + "parity", serialSettings->parity);
    settings.setValue (path + "stringParity", serialSettings->stringParity);
    settings.setValue (path + "stopBits", serialSettings->stopBits);
    settings.setValue (path + "stringStopBits", serialSettings->stringStopBits);
    settings.setValue (path + "flowControl", serialSettings->flowControl);
    settings.setValue (path + "stringFlowControl", serialSettings->stringFlowControl);
    qDebug() << __PRETTY_FUNCTION__ << "name" << serialSettings->name;
    qDebug() << __PRETTY_FUNCTION__ << "baudRate" << serialSettings->baudRate;
    qDebug() << __PRETTY_FUNCTION__ << "dataBits" << serialSettings->dataBits;
    qDebug() << __PRETTY_FUNCTION__ << "parity" << serialSettings->parity;
    qDebug() << __PRETTY_FUNCTION__ << "stringParity" << serialSettings->stringParity;
    qDebug() << __PRETTY_FUNCTION__ << "stopBits" << serialSettings->stopBits;
    qDebug() << __PRETTY_FUNCTION__ << "stringStopBits" << serialSettings->stringStopBits;
    qDebug() << __PRETTY_FUNCTION__ << "flowControl" << serialSettings->flowControl;
    qDebug() << __PRETTY_FUNCTION__ << "stringFlowControl" << serialSettings->stringFlowControl;
}
