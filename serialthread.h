/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPort>

//#if WINDOWS
//#define ALT_MODE  1
//#else
#define ALT_MODE  0
//#endif

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
    qint64 write(QByteArray data, const QString &lineEnding = "");
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
    QString parityStr();

    bool setStopBits(QSerialPort::StopBits stopBits);
    QSerialPort::StopBits stopBits();

    bool setFlowControl(QSerialPort::FlowControl flowControl);
    QSerialPort::FlowControl flowControl();
    QString flowControlStr();

    bool setDataTerminalReady(bool set);
    bool isDataTerminalReady();

    bool setRequestToSend(bool set);
    bool isRequestToSend();

    QSerialPort::PinoutSignals pinoutSignals();

    QString errorString();

    bool isOpen();

    QByteArray readAll(int timeout = 0);
    void recreatePort();

signals:
    void error(QSerialPort::SerialPortError);
    void readyRead();
    void progress(QString message, int percent);
    void finish();
    void pinoutSignalsChanged(QSerialPort::PinoutSignals pinoutSignals);

public slots:
    void abortSend();

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
#if ALT_MODE == 0
    QWaitCondition m_commandEvent; /**< Thread waits for this condition. */
#endif
    int m_delayAfterBytes_ms;   /**< After sending a byte this delay will be applied. */
    int m_delayAfterChr_ms;
    /** After this character m_delayAfterChr_ms microseconds delay will be applied instead of m_delayAfterBytes_ms.
     * This is usually a new line charater (CR, LF). */
    QByteArray m_chr;
    QSerialPort::PinoutSignals m_pinoutSignals;
    qint32 m_baudRateInput;
    qint32 m_baudRateOutput;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;
};

#endif // SERIALTHREAD_H
