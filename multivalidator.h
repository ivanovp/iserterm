/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef MULTIVALIDATOR_H
#define MULTIVALIDATOR_H

#include <QDebug>
#include <QValidator>

#include "multistring.h"

/**
 * @brief The MultiValidator class
 * Validates ASCII, hexadecimal, decimal or binary string
 */
class MultiValidator : public QValidator
{
public:
    MultiValidator(QObject * parent);
    virtual void fixup(QString &input) const;
    virtual State validate(QString &input, int &pos) const;
    void setMode(Multistring::mode_t mode);
    Multistring::mode_t getMode() const { return m_mode; }
private:
    Multistring::mode_t m_mode;
};

#endif // MULTIVALIDATOR_H
