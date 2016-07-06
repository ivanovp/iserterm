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
class Multistring
{
public:
    typedef enum
    {
        ASCII = 1,
        Binary = 2,
        Decimal = 10,
        Hexadecimal = 16
    } mode_t;

public:
    Multistring(QString str = "", mode_t mode = ASCII);
    ~Multistring();

    QString getString() const { return m_str; }
    bool setString(const QString& str);
    bool setString(const QString& str, mode_t mode);

    QByteArray getByteArray(bool *ok = NULL);
    void setByteArray(const QByteArray& arr);

    void setMode(mode_t mode);
    mode_t getMode() const { return m_mode; }

    bool getUpcase() const;
    void setUpcase(bool upcase = true);

protected:
    int getWidth() const;
    int getBase() const { return static_cast<int> (m_mode); }

private:
    QString m_str;
    mode_t m_mode;
    bool m_upcase;
};

#endif // MULTISTRING_H
