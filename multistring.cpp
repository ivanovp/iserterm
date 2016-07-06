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
    int base = getBase();
    int width = getWidth();

    m_str.clear();

    if (base > 1)
    {
        /* Number */
        foreach(char c, arr)
        {
            if (m_str.length() > 0)
            {
                m_str += " ";
            }
            m_str += QString("%1").arg(static_cast<int> (c), width, base, QChar('0'));
        }
        qDebug() << __PRETTY_FUNCTION__ << m_str;
    }
    else
    {
        /* ASCII data */
        m_str = QString (arr);
    }
}

QByteArray Multistring::getByteArray()
{
    QByteArray arr;
    QString str = m_str;
    int base = getBase();
    int width = getWidth();

    if (base > 1)
    {
        /* Number */
        str.remove(' ');
        while (str.length() > 1)
        {
            char c;
            bool ok;
            QString hexStr = str.left(width);
            c = hexStr.toInt(&ok, base);
            if (ok)
            {
                arr.append(c);
            }
            else
            {
                qWarning() << "Cannot convert string to number!";
            }
            str.remove(0, width);
        }
    }
    else
    {
        /* ASCII data */
        arr = str.toLocal8Bit();
    }
    qDebug() << __PRETTY_FUNCTION__ << arr;

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

int Multistring::getWidth() const
{
    int width = 0;

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

    return width;
}

