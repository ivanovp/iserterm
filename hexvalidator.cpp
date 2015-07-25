/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include <QDebug>
#include <QValidator>

#include "hexvalidator.h"

HexValidator::HexValidator(QObject * parent) : QValidator(parent)
{
}

void HexValidator::fixup(QString &input) const
{
    QString str;
    int index = 0;

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

    input = str;
}

HexValidator::State HexValidator::validate(QString &input, int &pos) const
{
    qDebug() << __PRETTY_FUNCTION__ << input << pos;
    if (!input.isEmpty())
    {
        const int char_pos = pos - input.left(pos).count(' ');
        int chars = 0;
        fixup(input);

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
    }
    return QValidator::Acceptable;
}
