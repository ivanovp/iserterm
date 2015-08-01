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
    typedef enum
    {
        LINE_rx,
        LINE_tx,
        LINE_dsr,
        LINE_cts,
        LINE_dcd,
        LINE_ri,
        LINE_dtr,
        LINE_rts,
        LINE_brk
    } lines_t;
    explicit SerialThread(QObject *parent = 0);
    ~SerialThread();

    void run();
    void stop(int timeout = 0);
    QSerialPort *getSerialPort();
    qint64 write(const QByteArray& data);
    qint64 write(const char *data, qint64 len);

    int getDelayAfterBytes_ms() const;
    void setDelayAfterBytes_ms(int delayAfterBytes_ms);

    int getDelayAfterChr_ms() const;
    QByteArray getChr() const;
    void setDelayAfterChr_ms(int delayAfterChr_ms, QByteArray chr);

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

    QSerialPort::PinoutSignals pinoutSignals();

    QString errorString();

    bool isOpen();

    QByteArray readAll();

signals:
    void error(QSerialPort::SerialPortError);
    void readyRead();
    void progress(QString message, int percent);
    void finished();
    void pinoutSignalsChanged(QSerialPort::PinoutSignals pinoutSignals);

public slots:

protected:
   void processCommand();

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
    int m_writeDataLength;
    int m_writeDataSent;
    QByteArray m_readData;      /**< Received data */
    bool m_running;             /**< Thread is running, used to stop thread gently. */
    QMutex m_mutex;             /**< Mutex to protect m_writeData, m_readData. */
    QWaitCondition m_commandEvent; /**< Thread waits for this condition. */
    int m_delayAfterBytes_ms;   /**< After sending a byte this delay will be applied. */
    int m_delayAfterChr_ms;
    QByteArray m_chr;           /**< After this character m_delayAfterChr_ms microseconds delay will be applied instead of m_delayAfterBytes_ms. */
    QSerialPort::PinoutSignals m_pinoutSignals;
};

#endif // SERIALTHREAD_H
