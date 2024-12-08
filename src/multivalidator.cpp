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
#ifdef __GNUC__
                if (std::isxdigit(ch.toLatin1()))
#else
                if (isxdigit(ch.toLatin1()))
#endif
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
#ifdef __GNUC__
                if (std::isdigit(ch.toLatin1()))
#else
                if (isdigit(ch.toLatin1()))
#endif
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
    const int char_pos = pos - input.left(pos).count(' ');
    int chars = 0;
    MultiValidator::State state = QValidator::Acceptable;
    QString str = input;
    int base = static_cast<int> (m_mode);
    int width = 3; /* only when base is 10 */

    if (!input.isEmpty())
    {
        fixup(input);

        switch (m_mode)
        {
            case Multistring::Decimal:
                str.remove(' ');
                while (str.length() > 1)
                {
                    int n;
                    bool ok;
                    QString hexStr = str.left(width);
                    n = hexStr.toInt(&ok, base);
                    if (!ok || n > 255)
                    {
                        state = QValidator::Invalid;
                    }
                    str.remove(0, width);
                }
                /* Fall through! */

            default:
            case Multistring::Hexadecimal:
            case Multistring::Binary:
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

            case Multistring::ASCII:
                break;
        }
    }

    return state;
}

void MultiValidator::setMode(Multistring::mode_t mode)
{
    m_mode = mode;
}
