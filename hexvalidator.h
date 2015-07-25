/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef HEXVALIDATOR_H
#define HEXVALIDATOR_H

#include <QDebug>
#include <QValidator>

/**
 * @brief The HexValidator class
 * Validates hexadecimal string
 */
class HexValidator : public QValidator
{
public:
    HexValidator(QObject * parent);
    virtual void fixup(QString &input) const;
    virtual State validate(QString &input, int &pos) const;
};

#endif // HEXVALIDATOR_H
