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

signals:

public slots:

protected:
    QIODevice* m_device;
    QByteArray m_data;      /**< Data to send */
    bool m_running;
    QMutex m_mutex;
    QWaitCondition m_dataAdded;
    int m_delayAfterBytes_us;
};

#endif // SERIALTHREAD_H
