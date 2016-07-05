/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include <QDebug>
#include <QValidator>

#include "hexvalidator.h"

HexValidator::HexValidator(QObject * parent) : QValidator(parent)
  , m_mode(Hexadecimal)
{
}

void HexValidator::fixup(QString &input) const
{
    QString str;
    int index = 0;

    switch (m_mode)
    {
        default:
        case Hexadecimal:
            foreach (QChar ch, input)
            {
                if (std::isxdigit(ch.toLatin1()))
                {
                    if (index != 0 && (index & 1) == 0)
                    {
                        str += ' ';
                    }

                    str += ch.toUpper();
                    index++;
                }
            }
            break;

        case Decimal:
            foreach (QChar ch, input)
            {
                if (std::isdigit(ch.toLatin1()))
                {
                    if (index != 0 && (index % 3) == 0)
                    {
                        str += ' ';
                    }

                    str += ch.toUpper();
                    index++;
                }
            }
            break;

        case Binary:
            break;

        case ASCII:
            break;
    }

    input = str;
}

HexValidator::State HexValidator::validate(QString &input, int &pos) const
{
//    qDebug() << __PRETTY_FUNCTION__ << input << pos;
    const int char_pos = pos - input.left(pos).count(' ');
    int chars = 0;

    if (!input.isEmpty())
    {
        fixup(input);

        switch (m_mode)
        {
            default:
            case Hexadecimal:
                pos = 0;

                while (chars != char_pos)
                {
                    if (input[pos] != ' ')
                    {
                        chars++;
                    }
                    pos++;
                }

                if (input[pos] == ' ')
                {
                    pos++;
                }
                break;

            case Decimal:
                break;

            case Binary:
                break;

            case ASCII:
                break;
        }
    }
    return QValidator::Acceptable;
}

void HexValidator::setMode(HexValidator::mode_t mode)
{
    m_mode = mode;
}
