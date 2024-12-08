/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/
#include "shiftdeleventfilter.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QComboBox>

bool ShiftDelEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key::Key_Delete || keyEvent->key() == Qt::Key::Key_Clear)
                && keyEvent->modifiers() == Qt::ShiftModifier)
        {
            QComboBox *combobox = dynamic_cast<QComboBox *>(obj);
//            qDebug() << "combobox:" << combobox;
            if (!combobox)
            {
                combobox = dynamic_cast<QComboBox *>(parent());
//                qDebug() << "combobox2:" << combobox;
            }
            if (combobox)
            {
                combobox->removeItem(combobox->currentIndex());
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}
