/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include <QDebug>
#include <QValidator>

#include "multivalidator.h"

MultiValidator::MultiValidator(QObject * parent) : QValidator(parent)
  , m_mode(Multistring::Hexadecimal)
{
}

void MultiValidator::fixup(QString &input) const
{
    QString str;
    int index = 0;

    switch (m_mode)
    {
        default:
        case Multistring::Hexadecimal:
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

        case Multistring::Decimal:
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

        case Multistring::Binary:
            foreach (QChar ch, input)
            {
                if (ch.toLatin1() == '0' || ch.toLatin1() == '1')
                {
                    if (index != 0 && (index % 8) == 0)
                    {
                        str += ' ';
                    }

                    str += ch.toUpper();
                    index++;
                }
            }
            break;

        case Multistring::ASCII:
            str = input;
            break;
    }

    input = str;
}

MultiValidator::State MultiValidator::validate(QString &input, int &pos) const
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
            case Multistring::Hexadecimal:
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

            case Multistring::Decimal:
                break;

            case Multistring::Binary:
                break;

            case Multistring::ASCII:
                break;
        }
    }
    return QValidator::Acceptable;
}

void MultiValidator::setMode(Multistring::mode_t mode)
{
    m_mode = mode;
}
