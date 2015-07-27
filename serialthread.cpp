/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#include <QMutexLocker>
#include <QIODevice>
#include <QDebug>

#include "serialthread.h"

SerialThread::SerialThread(QObject *parent)
    : QThread(parent)
    , m_device(NULL)
    , m_delayAfterBytes_us(1000)
    , m_delayAfterChr_us(1000)
{
}

void SerialThread::run()
{
    m_running = true;
    while (m_running)
    {
        m_mutex.lock();
        /* Unlocks mutex and waits for event! */
        m_dataAdded.wait(&m_mutex);
        /* Mutex locked and data received */
//        qDebug() << __PRETTY_FUNCTION__ << "sending" << m_data.length () << "bytes";
        /* Send data while thread should run */
        while (m_data.length () > 0 && m_running)
        {
            QByteArray c = m_data.left (1);
            m_data.remove (0, 1);
//            qDebug() << __PRETTY_FUNCTION__ << "sending" << c;
            m_device->write(c);
            m_mutex.unlock ();
            if (m_chr.length() > 0 && c == m_chr)
            {
                usleep (m_delayAfterChr_us);
            }
            else
            {
                usleep (m_delayAfterBytes_us);
            }
            m_mutex.lock ();
        }
        m_mutex.unlock ();
    }
}

void SerialThread::stop(int timeout)
{
    m_running = false;
    m_dataAdded.wakeAll();
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

void SerialThread::setSerialDevice(QIODevice *device)
{
    m_device = device;
}

/**
 * @brief SerialThread::addData
 * Add data to queue. Data will be sent with specified delay between bytes.
 * @param data Data to add to queue.
 */
void SerialThread::addData(const QByteArray &data)
{
    QMutexLocker mutexLocker(&m_mutex);
//    qDebug() << __PRETTY_FUNCTION__ << "adding" << data.length () << "bytes";
    m_data.append(data);
    m_dataAdded.wakeAll();
}

int SerialThread::getDelayAfterBytes_us() const
{
    return m_delayAfterBytes_us;
}

void SerialThread::setDelayAfterBytes_us(int delayAfterBytes_us)
{
    m_delayAfterBytes_us = delayAfterBytes_us;
}

int SerialThread::getDelayAfterChr_us() const
{
    return m_delayAfterChr_us;
}

QByteArray SerialThread::getChr() const
{
    return m_chr;
}

int SerialThread::setDelayAfterChr_us(int delayAfterChr_us, QByteArray chr)
{
    m_delayAfterChr_us = delayAfterChr_us;
    m_chr = chr;
}

