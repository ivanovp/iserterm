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
#include <QMenu>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_dataSizeLimit(32 * 1024 * 1024)
    , m_localEchoEnabled(false)
    , m_updateEnabled(true)
    , m_displayTimestampEnabled(false)
    , m_displayHexValuesEnabled(false)
    , m_lineEnding("\r\n")
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
    bgcolordef = QColor (Qt::black).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    QVariant fgcolordef;
    fgcolordef = QColor (Qt::green).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
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
    m_data.append(data);
    if (m_data.length () > m_dataSizeLimit)
    {
        /* Remove unwanted bytes */
        m_data.remove (0, m_data.length () - m_dataSizeLimit);
    }
    if (m_updateEnabled)
    {
        QScrollBar *bar = verticalScrollBar();
        /* Check if slider is scrolled to down */
        bool scrollToEnd = bar->sliderPosition() == bar->maximum();

        appendDataToConsole (data, scrollToEnd);
    }
}

void Console::clear()
{
    QPlainTextEdit::clear ();
    m_data.clear ();
}

bool Console::isLocalEchoEnabled() const
{
    return m_localEchoEnabled;
}

void Console::setLocalEchoEnabled(bool localEchoEnabled)
{
    m_localEchoEnabled = localEchoEnabled;
}

bool Console::isUpdateEnabled() const
{
    return m_updateEnabled;
}

void Console::setUpdateEnabled(bool updateEnabled)
{
    if (!m_updateEnabled && updateEnabled)
    {
        rebuildConsole ();
    }
    m_updateEnabled = updateEnabled;
}

QString Console::getLineEnding() const
{
    return m_lineEnding;
}

void Console::setLineEnding(const QString &lineEnding)
{
    m_lineEnding = lineEnding;
}

bool Console::isDisplayHexValuesEnabled() const
{
    return m_displayHexValuesEnabled;
}

void Console::setDisplayHexValuesEnabled(bool displayHexValuesEnabled)
{
    m_displayHexValuesEnabled = displayHexValuesEnabled;
}

bool Console::isDisplayTimestampEnabled() const
{
    return m_displayTimestampEnabled;
}

void Console::setDisplayTimestampEnabled(bool displayTimestampEnabled)
{
    m_displayTimestampEnabled = displayTimestampEnabled;
}

int Console::getDataSizeLimit() const
{
    return m_dataSizeLimit;
}

void Console::setDataSizeLimit(int dataSizeLimit)
{
    m_dataSizeLimit = dataSizeLimit;
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
            if (m_localEchoEnabled)
            {
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                QPlainTextEdit::keyPressEvent(e);
            }
            emit getData(e->text().toLocal8Bit());
    }
}

#if 0
void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}
#endif

void Console::contextMenuEvent(QContextMenuEvent *e)
{
    const bool showTextSelectionActions = textInteractionFlags() & (Qt::TextEditable | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    QMenu *menu = new QMenu;
    QAction *a;

    if (showTextSelectionActions)
    {
        a = menu->addAction(tr("&Copy"), this, SLOT(copy()), QKeySequence::Copy);
        a->setEnabled(textCursor().hasSelection());
    }

    if (textInteractionFlags() & Qt::TextEditable)
    {
#if !defined(QT_NO_CLIPBOARD)
        a = menu->addAction(tr("&Paste"), this, SLOT(paste()), QKeySequence::Paste);
        a->setEnabled(canPaste());
#endif
    }

    if (showTextSelectionActions)
    {
        menu->addSeparator();
        a = menu->addAction(tr("Select All"), this, SLOT(selectAll()), QKeySequence::SelectAll);
        a->setEnabled(!document()->isEmpty());
    }

    menu->exec(mapToGlobal(e->pos()));
    delete menu;
}

void Console::appendDataToConsole(const QByteArray &data, bool scrollToEnd)
{
    QByteArray data2;
    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    data2 = data;
    data2.replace (m_lineEnding, QByteArray("\n"));
//    if (m_lineEnding.length () > 1)
    if (m_lineEnding == "\r\n" || m_lineEnding == "\n\r")
    {
        /* If '\r' remains in buffer, remove them. '\n' expected in next buffer */
        data2.replace (QString (m_lineEnding[0]), QByteArray());
    }
//    if (strchr (data2, '\r') != 0)
//    {
//        qDebug() << "Line ending found!" << data;
//        qDebug() << data2;
//    }
    insertPlainText(QString(data2));

    if (scrollToEnd)
    {
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

void Console::rebuildConsole()
{
    QPlainTextEdit::clear ();
    appendDataToConsole (m_data, true);
}
