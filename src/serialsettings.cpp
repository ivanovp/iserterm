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
#include <QDataStream>

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

// FIXME it does not work
SerialSettings::operator QString() const
{
    QByteArray byteArray;
    QDataStream stream(byteArray);
    stream << m_serialSettings;
    // qDebug() << __PRETTY_FUNCTION__ << byteArray;

    return QString(byteArray);
}

QString SerialSettings::toVerboseString() const
{
    QString str;
    str = "Port: " + m_serialSettings.name + NATIVE_LINEENDNG;
    str += "Baud rate: " + QVariant(static_cast<int>(m_serialSettings.baudRate)).toString() + NATIVE_LINEENDNG;
    str += "Data bits: " + QVariant(static_cast<int>(m_serialSettings.dataBits)).toString() + NATIVE_LINEENDNG;
    str += "Parity bits: " + m_serialSettings.stringParity + NATIVE_LINEENDNG;
    str += "Stop bits: " + m_serialSettings.stringStopBits + NATIVE_LINEENDNG;
    str += "Flow control: " + m_serialSettings.stringFlowControl + NATIVE_LINEENDNG;

    return str;
}

QString SerialSettings::toString() const
{
    QString str;
    str = m_serialSettings.name;
    str += ", " + QVariant(static_cast<int>(m_serialSettings.baudRate)).toString() + ", ";
    str += QVariant(static_cast<int>(m_serialSettings.dataBits)).toString();
    str += m_serialSettings.stringParity[0];
    str += m_serialSettings.stringStopBits;
    str += ", " + m_serialSettings.stringFlowControl;

    return str;
}

void SerialSettings::loadSettings(SerialSettings::serialSettings_t *serialSettings, QString profileName)
{
    QSettings settings;
    QString path = "serial/";

    if (!profileName.isEmpty())
    {
        path = "profile/" + profileName + "/";
        qDebug() << __FUNCTION__ << "### profile" << profileName;
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
    qDebug() << __FUNCTION__ << toString();
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
        qDebug() << __FUNCTION__ << "### profile" << profileName;
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
    qDebug() << __FUNCTION__ << toString();
}

// FIXME it does not work
QDataStream &operator<<(QDataStream &out, const SerialSettings::serialSettings_t& s)
{
    out << "Port: " << s.name;
    out << "Baud rate: " << s.baudRate;
    out << "Data bits: " << s.dataBits;
    out << "Parity bits: " << s.stringParity;
    out << "Stop bits: " << s.stringStopBits;
    out << "Flow control: " << s.stringFlowControl;
    return out;
}

// QDataStream &operator>>(QDataStream &in, SerialSettings::serialSettings_t& s)
// {
//     in >> s.name >> s.baudRate;
//     return in;
// }

