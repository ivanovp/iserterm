/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define TOSTR(s)        XSTR(s)
#define XSTR(s)         #s

#ifdef QT_DEBUG
#define MY_ASSERT(x)    do { bool ok = (x); Q_ASSERT(ok); } while (0)
#else
#define MY_ASSERT(x)    x
#endif

#define CUSTOM_TEXT_NUM     6
#define NATIVE_LINEENDNG    "\n"

#define BACKSPACE       8u
#define DELETE          127u

#define CR              13u
#define LF              10u

#endif /* COMMON_H */
