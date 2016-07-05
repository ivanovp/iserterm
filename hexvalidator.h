/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef HEXVALIDATOR_H
#define HEXVALIDATOR_H

#include <QDebug>
#include <QValidator>

/**
 * @brief The HexValidator class
 * Validates hexadecimal, decimal or binary string
 */
class HexValidator : public QValidator
{
public:
    typedef enum
    {
        Hexadecimal = 0,
        Decimal,
        Binary,
        ASCII
    } mode_t;
public:
    HexValidator(QObject * parent);
    virtual void fixup(QString &input) const;
    virtual State validate(QString &input, int &pos) const;
    void setMode(mode_t mode);
    mode_t getMode() const { return m_mode; }
private:
    mode_t m_mode;
};

#endif // HEXVALIDATOR_H
