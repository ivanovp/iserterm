/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2024 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef SERIALSETTINGS_H
#define SERIALSETTINGS_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QDataStream>

class SerialSettings : public QObject
{
    Q_OBJECT

public:
    typedef struct
    {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
    } serialSettings_t;

    explicit SerialSettings(QObject *parent = 0);
    ~SerialSettings();

    serialSettings_t serialSettings() const;
    operator QString() const;

    QString toVerboseString() const;
    QString toString() const;

    void loadSettings(serialSettings_t *settings, QString profileName = "");
    void loadSettings(QString profileName = "") { loadSettings(&m_serialSettings, profileName); }
    void saveSettings(serialSettings_t *settings, QString profileName = "");
    void saveSettings(QString profileName = "") { saveSettings(&m_serialSettings, profileName); }

private slots:

public:
    serialSettings_t        m_serialSettings;
};

QDataStream &operator<<(QDataStream &out, const SerialSettings::serialSettings_t& s);
// QDataStream &operator>>(QDataStream &in, SerialSettings::serialSettings_t& s);

#endif // SERIALSETTINGS_H
