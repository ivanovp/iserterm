/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#include "multistring.h"

Multistring::Multistring()
{
}

Multistring::~Multistring()
{

}

void Multistring::setMode(Multistring::mode_t mode)
{
    if (m_mode != mode)
    {
        switch (m_mode)
        {

        }

        m_mode = mode;
    }
}

