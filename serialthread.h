/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPort>

/**
 * @brief The SerialThread class
 * Used to send serial data with delays between bytes.
 */
class SerialThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = 0);

    void run();
    void stop(int timeout = 0);
    QSerialPort *getSerialPort();
    qint64 write(const QByteArray& data);
    qint64 write(const char *data, qint64 len);

    int getDelayAfterBytes_us() const;
    void setDelayAfterBytes_us(int delayAfterBytes_us);

    int getDelayAfterChr_us() const;
    QByteArray getChr() const;
    void setDelayAfterChr_us(int delayAfterChr_us, QByteArray chr);

    void setPortName(const QString &name);
    QString portName();

    void setPort(const QSerialPortInfo &info);

    bool open(QSerialPort::OpenMode mode);
    void close();

    bool setBaudRate(qint32 baudRate, QSerialPort::Directions directions = QSerialPort::AllDirections);
    qint32 baudRate(QSerialPort::Directions directions = QSerialPort::AllDirections);

    bool setDataBits(QSerialPort::DataBits dataBits);
    QSerialPort::DataBits dataBits();

    bool setParity(QSerialPort::Parity parity);
    QSerialPort::Parity parity();

    bool setStopBits(QSerialPort::StopBits stopBits);
    QSerialPort::StopBits stopBits();

    bool setFlowControl(QSerialPort::FlowControl flowControl);
    QSerialPort::FlowControl flowControl();

    bool setDataTerminalReady(bool set);
    bool isDataTerminalReady();

    bool setRequestToSend(bool set);
    bool isRequestToSend();

    QString errorString();

    bool isOpen();

    QByteArray readAll();

signals:
    void error(QSerialPort::SerialPortError);
    void readyRead();
    void progress(QString message, int percent);
    void finished();

public slots:

protected:
    typedef enum
    {
        CMD_undefined,
        CMD_open,
        CMD_close,
        CMD_write,
        CMD_stop
    } command_t;
    command_t m_command;
    int m_commandParam;
    QSerialPort* m_serialPort;  /**< Serial device */
    QByteArray m_writeData;     /**< Data to send */
    bool m_running;             /**< Thread is running, used to stop thread gently. */
    QMutex m_mutex;             /**< Mutex to protect m_data. */
    QWaitCondition m_commandEvent; /**< Thread waits for this condition. */
    int m_delayAfterBytes_us;   /**< After sending a byte this delay will be applied. */
    int m_delayAfterChr_us;
    QByteArray m_chr;           /**< After this character m_delayAfterChr_us microseconds delay will be applied instead of m_delayAfterBytes_us. */
};

#endif // SERIALTHREAD_H
