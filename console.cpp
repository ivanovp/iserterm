/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
** This file is based on terminal example of Qt.
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "console.h"

#include <QScrollBar>

#include <QtCore/QDebug>
#include <QSettings>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , localEchoEnabled(false)
{
    document()->setMaximumBlockCount(1000);
    QSettings settings;
    QString fontStr = settings.value("console/font", "Monospace,12").toString();
    QFont font;
    font.fromString(fontStr);
    font.setStyleHint(QFont::TypeWriter);
    document()->setDefaultFont(font);
    QPalette p = palette();
    QVariant bgcolordef;
    bgcolordef = QColor (Qt::black).name(QColor::HexArgb); // Convert default color to "#rrggbb" format string
    QVariant fgcolordef;
    fgcolordef = QColor (Qt::green).name(QColor::HexArgb); // Convert default color to "#rrggbb" format string
    QColor bgcolor = settings.value("console/bgcolor", bgcolordef).toString();
    QColor fgcolor = settings.value("console/fgcolor", fgcolordef).toString();
    qDebug() << "bgcolor" << bgcolor;
    qDebug() << "fgcolor" << fgcolor;
    p.setColor(QPalette::Base, bgcolor);
    p.setColor(QPalette::Text, fgcolor);
    setPalette(p);
}

void Console::putData(const QByteArray &data)
{
    QScrollBar *bar = verticalScrollBar();
    bool scrolledToEnd = bar->sliderPosition() == bar->maximum();

    insertPlainText(QString(data));

    if (scrolledToEnd)
    {
      bar->setValue(bar->maximum());
    }
}

void Console::setLocalEchoEnabled(bool set)
{
    localEchoEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    default:
        if (localEchoEnabled)
        {
            QPlainTextEdit::keyPressEvent(e);
        }
        emit getData(e->text().toLocal8Bit());
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void Console::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}
