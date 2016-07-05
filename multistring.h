/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#ifndef MULTISTRING_H
#define MULTISTRING_H

#include <QString>

/*
 * It can store ASCII, hexadecimal, decimal and binary text and it can convert
 * them.
 */
class Multistring : public QString
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
    explicit Multistring();
    ~Multistring();

    void setMode(mode_t mode);
    mode_t getMode() const { return m_mode; }

private:
    mode_t m_mode;
};

#endif // MULTISTRING_H
