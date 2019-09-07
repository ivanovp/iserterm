/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#include <QMutexLocker>
#include <QSerialPort>
#include <QDebug>
#include <QElapsedTimer>

#include "common.h"
#include "serialthread.h"

Q_DECLARE_METATYPE(QSerialPort::SerialPortError)
Q_DECLARE_METATYPE(QSerialPort::PinoutSignals)

SerialThread::SerialThread(QObject *parent)
    : QThread(parent)
    , m_command(CMD_undefined)
    , m_serialPort(NULL)
    , m_writeDataLength(0)
    , m_writeDataSent(0)
    , m_delayAfterBytes_ms(1)
    , m_delayAfterChr_ms(1)
    , m_baudRateInput(115200)
    , m_baudRateOutput(115200)
    , m_dataBits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stopBits(QSerialPort::OneStop)
    , m_flowControl(QSerialPort::NoFlowControl)
{
    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");
    qRegisterMetaType<QSerialPort::PinoutSignals>("QSerialPort::PinoutSignals");
}

SerialThread::~SerialThread()
{
    m_running = false;
    if (isRunning())
    {
        stop(10);
    }
    delete m_serialPort;
    m_serialPort = NULL;
}

void SerialThread::run()
{
    Q_ASSERT(m_serialPort == NULL);
    m_running = true;
    recreatePort();

    while (m_running)
    {
        m_mutex.lock();
        /* Unlocks mutex and waits for event! */
#if ALT_MODE
        if (m_running && m_command != CMD_undefined)
#else
        if (m_running && m_commandEvent.wait(&m_mutex, 10))
#endif
        {
            /* Mutex locked and command received */
            processCommand();
        }
        if (m_running)
        {
            if (m_serialPort->isOpen())
            {
                if (m_serialPort->waitForReadyRead(10))
                {
//                    m_readData.append(m_serialPort->read(1024)); // read maximum 1024 byte only
                    m_readData.append(m_serialPort->readAll());
                    emit readyRead();
                }
                /* Check if CTS, RTS, etc. signals changed */
                QSerialPort::PinoutSignals pinoutSignals = m_serialPort->pinoutSignals();
                if (pinoutSignals != m_pinoutSignals)
                {
                    emit pinoutSignalsChanged(pinoutSignals);
                    m_pinoutSignals = pinoutSignals;
                }
            }
            else
            {
                msleep(10);
            }
        }
#if ALT_MODE
        else
        {
          msleep(10);
        }
#endif
        m_mutex.unlock();
    }
    delete m_serialPort;
    m_serialPort = NULL;
}

/**
 * @brief SerialThread::stop
 * Stops thread gently if possible. Othwerwise it terminates itself.
 *
 * @param timeout Wait in milliseconds before forcing stop.
 */
void SerialThread::stop(int timeout)
{
    m_running = false;
    m_command = CMD_stop;
#if ALT_MODE == 0
    m_commandEvent.wakeAll();
#endif
    if (timeout > 0)
    {
        if (!wait(timeout))
        {
            qDebug() << __PRETTY_FUNCTION__ << "terminating itself";
            terminate();
            wait();
        }
    }
}

QSerialPort *SerialThread::getSerialPort()
{
    Q_ASSERT(m_serialPort);
    return m_serialPort;
}

/**
 * @brief SerialThread::write
 * Add data to queue. Data will be sent with specified delay between bytes.
 * @param data Data to add to queue.
 * @param lineEnding Line ending to be converted. If empty: do not convert line ending.
 */
qint64 SerialThread::write(QByteArray data, const QString& lineEnding)
{
    QMutexLocker mutexLocker(&m_mutex);
//    qDebug() << __PRETTY_FUNCTION__ << "adding" << data.length () << "bytes";
    if (lineEnding.length())
    {
        data.replace(QString(NATIVE_LINEENDNG), lineEnding.toLocal8Bit());
    }
    m_writeData.append(data);
    m_command = CMD_write;
    m_commandParam = 0;
#if ALT_MODE == 0
    m_commandEvent.wakeAll();
#endif
    int length = data.length();
    m_writeDataLength += length;
    return length;
}

qint64 SerialThread::write(const char *data, qint64 len)
{
    QByteArray data_;
    for (int i = 0; i < len; i++)
    {
        data_.append(data[i]);
    }
    return write(data_);
}

int SerialThread::getDelayAfterBytes_ms() const
{
    return m_delayAfterBytes_ms;
}

void SerialThread::setDelayAfterBytes_ms(int delayAfterBytes_ms)
{
    m_delayAfterBytes_ms = delayAfterBytes_ms;
}

int SerialThread::getDelayAfterChr_ms() const
{
    return m_delayAfterChr_ms;
}

QByteArray SerialThread::getChr() const
{
    return m_chr;
}

void SerialThread::setDelayAfterChr_ms(int delayAfterChr_ms, QByteArray chr)
{
    m_delayAfterChr_ms = delayAfterChr_ms;
    m_chr = chr;
}

void SerialThread::setPortName(const QString &name)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_portName = name;
}

QString SerialThread::portName()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->portName();
}

void SerialThread::setPort(const QSerialPortInfo &info)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_serialPort->setPort(info);
}

bool SerialThread::open(QIODevice::OpenMode mode) //Q_DECL_OVERRIDE
{
    QMutexLocker mutexLocker(&m_mutex);
    m_command = CMD_open;
    m_commandParam = mode;
#if ALT_MODE == 0
    m_commandEvent.wakeAll();
#endif
    return true;
}

void SerialThread::close() //Q_DECL_OVERRIDE
{
    QMutexLocker mutexLocker(&m_mutex);
    m_command = CMD_close;
    m_commandParam = 0;
#if ALT_MODE == 0
    m_commandEvent.wakeAll();
#endif
}

bool SerialThread::setBaudRate(qint32 baudRate, QSerialPort::Directions directions)
{
    QMutexLocker mutexLocker(&m_mutex);
    if (directions == QSerialPort::AllDirections)
    {
      m_baudRateInput = baudRate;
      m_baudRateOutput = baudRate;
    }
    else if (directions == QSerialPort::Input)
    {
      m_baudRateInput = baudRate;
    }
    else if (directions == QSerialPort::Output)
    {
      m_baudRateOutput = baudRate;
    }
    return true;
//    return m_serialPort->setBaudRate(baudRate, directions);
}

qint32 SerialThread::baudRate(QSerialPort::Directions directions)
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->baudRate(directions);
}

bool SerialThread::setDataBits(QSerialPort::DataBits dataBits)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_dataBits = dataBits;
    return true;
//    return m_serialPort->setDataBits(dataBits);
}

QSerialPort::DataBits SerialThread::dataBits()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->dataBits();
}

bool SerialThread::setParity(QSerialPort::Parity parity)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_parity = parity;
    return true;
//    return m_serialPort->setParity(parity);
}

QSerialPort::Parity SerialThread::parity()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->parity();
}

QString SerialThread::parityStr()
{
    QString str;

    switch (m_serialPort->parity())
    {
        case QSerialPort::NoParity:
            str = tr("None");
            break;
        case QSerialPort::EvenParity:
            str = tr("Even");
            break;
        case QSerialPort::OddParity:
            str = tr("Odd");
            break;
        case QSerialPort::MarkParity:
            str = tr("Mark");
            break;
        case QSerialPort::SpaceParity:
            str = tr("Space");
            break;
        default:
            str = tr("Unknown");
            break;
    }

    return str;
}

bool SerialThread::setStopBits(QSerialPort::StopBits stopBits)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_stopBits = stopBits;
    return true;
//    return m_serialPort->setStopBits(stopBits);
}

QSerialPort::StopBits SerialThread::stopBits()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->stopBits();
}

bool SerialThread::setFlowControl(QSerialPort::FlowControl flowControl)
{
    QMutexLocker mutexLocker(&m_mutex);
    m_flowControl = flowControl;
    return true;
//    return m_serialPort->setFlowControl(flowControl);
}

QSerialPort::FlowControl SerialThread::flowControl()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->flowControl();
}

QString SerialThread::flowControlStr()
{
    QString str;

    switch (m_serialPort->flowControl())
    {
        case QSerialPort::NoFlowControl:
            str = tr("No handshake");
            break;
        case QSerialPort::HardwareControl:
            str = tr("RTS/CTS");
            break;
        case QSerialPort::SoftwareControl:
            str = tr("XON/XOFF");
            break;
        default:
            str = tr("Unknown");
            break;
    }

    return str;
}

bool SerialThread::setDataTerminalReady(bool set)
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->setDataTerminalReady(set);
}

bool SerialThread::isDataTerminalReady()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->isDataTerminalReady();
}

bool SerialThread::setRequestToSend(bool set)
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->setRequestToSend(set);
}

bool SerialThread::isRequestToSend()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->isRequestToSend();
}

QSerialPort::PinoutSignals SerialThread::pinoutSignals()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->pinoutSignals();
}

QString SerialThread::errorString()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->errorString();
}

bool SerialThread::isOpen()
{
    QMutexLocker mutexLocker(&m_mutex);
    return m_serialPort->isOpen();
}

QByteArray SerialThread::readAll(int timeout)
{
    QByteArray data;
    if (timeout == 0)
    {
        QMutexLocker mutexLocker(&m_mutex);
        data = m_readData;
        m_readData.clear();
    }
    else
    {
        if (m_mutex.tryLock(timeout))
        {
            data = m_readData;
            m_readData.clear();
            m_mutex.unlock();
        }
        else
        {
          qDebug() << __FUNCTION__ << "timeout";
        }
    }
    return data;
}

void SerialThread::recreatePort()
{
  /* On MingW32/Win7 serial port's handle cannot be reused.
   * "The handle is invalid" error occurs.
   */
  delete m_serialPort;
  m_serialPort = new QSerialPort;
  Q_ASSERT(m_serialPort);

  if (!m_serialPort)
  {
    qCritical() << __PRETTY_FUNCTION__ << "not enough memory";
    m_running = false;
  }
  else
  {
    MY_ASSERT(connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
                      SIGNAL(error(QSerialPort::SerialPortError))));
  }
}

void SerialThread::abortSend()
{
    qDebug() << __FUNCTION__;
    QMutexLocker mutexLocker(&m_mutex);
    m_writeData.clear();
}

void SerialThread::processCommand()
{
    bool progressSent = false;
    const int progressLimit_ms = 2000; /* 2 seconds */
    float progress_percent = 0.0f;

    if (m_command == CMD_write)
    {
        progress_percent = 0.0f;
        if (m_writeData.length())
        {
            if (m_writeDataLength * m_delayAfterBytes_ms >= progressLimit_ms)
            {
                progressSent = true;
                emit progress(QString(tr("Sending %1 bytes")).arg(m_writeDataLength), progress_percent);
            }
            /* Send data while thread should run */
            //qDebug() << __PRETTY_FUNCTION__ << m_writeData.length() << m_running << m_serialPort->isOpen() << m_serialPort->isWritable();
            while (m_writeData.length() > 0 && m_running && m_serialPort->isOpen() && m_serialPort->isWritable())
            {
                QByteArray c = m_writeData.left(1);
                m_writeData.remove(0, 1);
                m_mutex.unlock();
                //qDebug() << __PRETTY_FUNCTION__ << "sending" << c;
                m_serialPort->write(c);
                m_serialPort->flush();
                m_writeDataSent++;
                progress_percent = 100.0f * m_writeDataSent / m_writeDataLength;
                if (m_writeDataLength * m_delayAfterBytes_ms >= progressLimit_ms)
                {
                    progressSent = true;
                    emit progress(QString(tr("%1 bytes of %2 bytes sent")).arg(m_writeDataSent).arg(m_writeDataLength), progress_percent);
                }
                /* Character sent, delay for specified time. Meanwhile check if
                 * data can be received.
                 */
                int delay_ms = 0;
                if (m_chr.length() > 0 && c == m_chr)
                {
                    delay_ms = m_delayAfterChr_ms;
                }
                else
                {
                    delay_ms = m_delayAfterBytes_ms;
                }
                //qDebug() << __PRETTY_FUNCTION__ << "delay_ms" << delay_ms;
                QElapsedTimer timer;
                int elapsed_ms;
                timer.start();
                /* Check if data can be received and measure time of
                 * operation. waitForReadyRead() can block running
                 * up to delay_ms time.
                 */
                if (delay_ms && m_serialPort->waitForReadyRead(delay_ms))
                {
                    m_mutex.lock();
                    m_readData.append(m_serialPort->readAll());
                    m_mutex.unlock();
                    emit readyRead();
                }
                elapsed_ms = timer.elapsed();
                //qDebug() << __PRETTY_FUNCTION__ << "elapsed_ms" << elapsed_ms;
                if (elapsed_ms < delay_ms)
                {
                    /* Not enough time elapsed, another delay needed */
                    msleep (delay_ms - elapsed_ms);
                }
                m_mutex.lock();
            }
            if (progressSent)
            {
                emit progress(QString(tr("%1 bytes sent")).arg(m_writeDataSent), 100.0f);
            }
            emit finish();
            m_writeDataSent = 0;
            m_writeDataLength = 0;
        }
        m_command = CMD_undefined;
    }
    else if (m_command == CMD_open)
    {
        if (!m_serialPort->isOpen())
        {
#if WINDOWS
            /* Workaround for Windows */
            m_serialPort->setPortName(m_portName);
            m_serialPort->open(static_cast<QSerialPort::OpenMode>(m_commandParam));
            if (m_serialPort->isOpen())
            {
              m_serialPort->close();
              recreatePort();
            }
#endif
            m_serialPort->setPortName(m_portName);
            bool isOpened = m_serialPort->open(static_cast<QSerialPort::OpenMode>(m_commandParam));
            if (isOpened)
            {
              // Parameters shall be set after open on Qt 5.3!
#if 0
              // buggy on Windows 10 with PL2303 (not sure which one is guily)
              m_serialPort->setBaudRate(m_baudRateOutput, QSerialPort::Output);
              m_serialPort->setBaudRate(m_baudRateInput, QSerialPort::Input);
#else
              m_serialPort->setBaudRate(m_baudRateOutput);
#endif
              m_serialPort->setDataBits(m_dataBits);
              m_serialPort->setParity(m_parity);
              m_serialPort->setStopBits(m_stopBits);
              m_serialPort->setFlowControl(m_flowControl);
            }
            else
            {
              qCritical() << __PRETTY_FUNCTION__ << "cannot open port!";
            }
        }
        m_command = CMD_undefined;
    }
    else if (m_command == CMD_close || m_command == CMD_stop)
    {
        if (m_serialPort->isOpen())
        {
            m_serialPort->close();
        }
        m_command = CMD_undefined;
    }
    else if (m_command == CMD_undefined)
    {
        qDebug() << __PRETTY_FUNCTION__ << "undefined command";
    }
    else
    {
        qCritical() << __PRETTY_FUNCTION__ << "unknown command:" << (int)m_command;
        Q_ASSERT(0);
    }
}
