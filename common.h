#ifndef COMMON_H
#define COMMON_H

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define TOSTR(s)        XSTR(s)
#define XSTR(s)         #s

#ifdef QT_DEBUG
#define _ASSERT(x)   do { bool ok = (x); Q_ASSERT(ok); } while (0)
#else
#define _ASSERT(x)   x
#endif

#endif /* COMMON_H */
