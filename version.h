#ifndef VERSION_H
#define VERSION_H

#include "common.h"

#define VER_PRODUCT_MAJOR           0
#define VER_PRODUCT_MINOR           1
#define VER_PRODUCT_RELEASE         3

#ifdef QT_DEBUG
#define VER_PRODUCT_BUILD           99
#define DEBUG_STR " DEBUG"
#else
#define VER_PRODUCT_BUILD           0
#define DEBUG_STR ""
#endif


#define VER_PRODUCTVERSION          VER_PRODUCT_MAJOR,VER_PRODUCT_MINOR,VER_PRODUCT_RELEASE,VER_PRODUCT_BUILD
#define VER_PRODUCTVERSION_STR      TOSTR(VER_PRODUCT_MAJOR) "." TOSTR(VER_PRODUCT_MINOR) "." TOSTR(VER_PRODUCT_RELEASE) "." TOSTR(VER_PRODUCT_BUILD) "\0"

#define VER_FILEVERSION             VER_PRODUCTVERSION
#define VER_FILEVERSION_STR         VER_PRODUCTVERSION_STR

#define VER_COMPANYNAME_STR         "Peter Ivanov"
#define VER_PRODUCTNAME_STR         "iSerTerm" DEBUG_STR
#define VER_FILEDESCRIPTION_STR     VER_PRODUCTNAME_STR
#define VER_INTERNALNAME_STR        VER_PRODUCTNAME_STR
#define VER_LEGALCOPYRIGHT_STR      "Copyright (c) 2015 Peter Ivanov"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "iSerTerm"

#define VER_COMPANYDOMAIN_STR       "ivanov.eu"

#endif // VERSION_H
