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

class QIODevice;

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
    void setSerialDevice(QIODevice* device);
    void addData(const QByteArray& data);

    int getDelayAfterBytes_us() const;
    void setDelayAfterBytes_us(int delayAfterBytes_us);

    int getDelayAfterChr_us() const;
    QByteArray getChr() const;
    int setDelayAfterChr_us(int delayAfterChr_us, QByteArray chr);

signals:

public slots:

protected:
    QIODevice* m_device;        /**< Serial device */
    QByteArray m_data;          /**< Data to send */
    bool m_running;             /**< Thread is running, used to stop thread gently. */
    QMutex m_mutex;             /**< Mutex to protect m_data. */
    QWaitCondition m_dataAdded; /**< Thread waits for this condition. */
    int m_delayAfterBytes_us;   /**< After sending a byte this delay will be applied. */
    int m_delayAfterChr_us;
    QByteArray m_chr;           /**< After this character m_delayAfterChr_us microseconds delay will be applied instead of m_delayAfterBytes_us. */
};

#endif // SERIALTHREAD_H
