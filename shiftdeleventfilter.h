/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#ifndef SHIFTDELEVENTFILTER_H
#define SHIFTDELEVENTFILTER_H

#include <QObject>

/**
 * @brief The ShiftDelEventFilter class
 * Unnecessary history elements can be deleted by pressing Shift-Del in a
 * combobox.
 */
class ShiftDelEventFilter : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // SHIFTDELEVENTFILTER_H
