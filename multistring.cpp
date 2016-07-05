/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#include "multistring.h"

#include <QDebug>

Multistring::Multistring(QString str, mode_t mode)
    : m_str( str )
    , m_mode( mode )
{
}

Multistring::~Multistring()
{
}

bool Multistring::setString(const QString &str)
{
    m_str = str;
    return true; // FIXME add validity check
}

bool Multistring::setString(const QString &str, mode_t mode)
{
    m_mode = mode;
    m_str = str;
    return true; // FIXME add validity check
}

void Multistring::setByteArray(const QByteArray &arr)
{
    int base = static_cast<int> (m_mode);
    int width = 0;

    m_str.clear();

    switch (m_mode)
    {
        default:
        case Hexadecimal:
            width = 2;
            break;

        case Decimal:
            width = 3;
            break;

        case Binary:
            width = 8;
            break;

        case ASCII:
            break;
    }

    if (base > 1)
    {
        foreach(char c, arr)
        {
            if (m_str.length() > 0)
            {
                m_str += " ";
            }
//            m_str += QString::number(static_cast<int> (c), base);
            m_str += QString("%1").arg(static_cast<int> (c), width, base);
        }
    }
    else
    {
        m_str = arr;
    }
}

QByteArray Multistring::getByteArray()
{
    QByteArray arr;
    QString str = m_str;

    switch (m_mode)
    {
        default:
        case Hexadecimal:
            str.remove(' ');
            while (str.length() > 1)
            {
                char c;
                bool ok;
                QString hexStr = str.left(2);
                c = hexStr.toInt(&ok, 16);
                if (ok)
                {
                    arr.append(c);
                }
                else
                {
                    qWarning() << "Cannot convert string to number!";
                }
                str.remove(0, 2);
            }
            break;

        case Decimal:
            break;

        case Binary:
            break;

        case ASCII:
            arr = str.toLocal8Bit();
            break;
    }

    return arr;
}

/**
 * @brief Multistring::setMode Convert string to another form.
 * @param mode
 */
void Multistring::setMode(Multistring::mode_t mode)
{
    if (m_mode != mode)
    {
        qDebug() << __PRETTY_FUNCTION__ << "conversion needed from mode" << m_mode << "to mode" << mode;
        QByteArray arr = getByteArray();
        m_mode = mode;
        setByteArray(arr);
    }
}

bool Multistring::getUpcase() const
{
    return m_upcase;
}

void Multistring::setUpcase(bool upcase)
{
    m_upcase = upcase;
}

