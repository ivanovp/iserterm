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
#include "common.h"

#include <QScrollBar>
#include <QApplication>
#include <QClipboard>

#include <QtCore/QDebug>
#include <QSettings>
#include <QMenu>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_dataSizeLimit(1 * 1024 * 1024) /* 1 MiB by default */
    , m_localEchoEnabled(false)
    , m_updateEnabled(true)
    , m_displayTimestampEnabled(false)
    , m_displayHexValuesEnabled(false)
    , m_hexWrap(16)
    , m_lineEndingRx("\r\n")
    , m_lineEndingTx("\r")
{
    document()->setMaximumBlockCount(10000);
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
    fgcolordef = QColor (Qt::lightGray).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    QColor bgcolor = settings.value("console/bgcolor", bgcolordef).toString();
    QColor fgcolor = settings.value("console/fgcolor", fgcolordef).toString();
//    qDebug() << "bgcolor" << bgcolor;
//    qDebug() << "fgcolor" << fgcolor;
    p.setColor(QPalette::Base, bgcolor);
    p.setColor(QPalette::Text, fgcolor);
    setPalette(p);

    m_keyMap.insert(Qt::Key_Backspace,                      KeyMap(false, "\x08"));
    m_keyMap.insert(Qt::Key_Delete,                         KeyMap(false, "\x7F"));
    m_keyMap.insert(Qt::Key_Return,                         KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter,                          KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter | Qt::KeypadModifier,     KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_C | Qt::ControlModifier,        KeyMap(true, ""));
    m_keyMap.insert(Qt::Key_V | Qt::ControlModifier,        KeyMap(true, ""));
    m_keyMap.insert(Qt::Key_A | Qt::ControlModifier,        KeyMap(true, ""));
}

void Console::putData(const QByteArray &data)
{
    if (m_updateEnabled)
    {
        QScrollBar *bar = verticalScrollBar();
        /* Check if slider is scrolled to down */
        bool scrollToEnd = bar->sliderPosition() == bar->maximum();

        appendDataToConsole (data, scrollToEnd);
    }

    m_data.append(data);
    if (m_data.length () > m_dataSizeLimit)
    {
        /* Remove unwanted bytes */
        m_data.remove (0, m_data.length () - m_dataSizeLimit);
    }
}

void Console::clear()
{
    qDebug() << __PRETTY_FUNCTION__;
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
        m_updateEnabled = updateEnabled;
        rebuildConsole ();
    }
}

QString Console::getLineEndingRx() const
{
    return m_lineEndingRx;
}

void Console::setLineEndingRx(const QString &lineEndingRx)
{
    m_lineEndingRx = lineEndingRx;
}

QString Console::getLineEndingTx() const
{
    return m_lineEndingTx;
}

void Console::setLineEndingTx(const QString &lineEndingTx)
{
    m_lineEndingTx = lineEndingTx;
    m_keyMap.insert(Qt::Key_Return,                         KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter,                          KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter | Qt::KeypadModifier,     KeyMap(true, m_lineEndingTx));
}

void Console::paste()
{
    if (m_localEchoEnabled)
    {
        moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        QPlainTextEdit::paste();
    }
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();
    emit getData(originalText.toLocal8Bit ());
}

bool Console::isDisplayHexValuesEnabled() const
{
    return m_displayHexValuesEnabled;
}

void Console::setDisplayHexValuesEnabled(bool displayHexValuesEnabled)
{
    if (m_displayHexValuesEnabled != displayHexValuesEnabled)
    {
        m_displayHexValuesEnabled = displayHexValuesEnabled;
        rebuildConsole();
    }
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

int Console::getDisplaySize() const
{
    return document ()->maximumBlockCount ();
}

void Console::setDisplaySize(int displaySize)
{
    document ()->setMaximumBlockCount (displaySize);
}

int Console::getHexWrap() const
{
    return m_hexWrap;
}

void Console::setHexWrap(int hexWrap)
{
    Q_ASSERT(hexWrap >= 1 && hexWrap <= 1024);
    if (hexWrap >= 1 && hexWrap <= 1024)
    {
        if (m_hexWrap != hexWrap)
        {
            m_hexWrap = hexWrap;
            if (isDisplayHexValuesEnabled ())
            {
                rebuildConsole ();
            }
        }
    }
}

void Console::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    int modifier = static_cast<int> (e->modifiers ());
//    qDebug() << __PRETTY_FUNCTION__ << key;
    if (modifier == Qt::ControlModifier && (key == Qt::Key_C || key == Qt::Key_V || key == Qt::Key_A))
    {
        if (key == Qt::Key_C || key == Qt::Key_A)
        {
            /* Ctrl-C (Copy) and Ctrl-A (Select all) can be forwarded anytime */
            QPlainTextEdit::keyPressEvent(e);
        }
        else
        {
            /* Ctrl-V (Paste) */
            if (m_localEchoEnabled)
            {
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                QPlainTextEdit::keyPressEvent(e);
            }
            QClipboard *clipboard = QApplication::clipboard();
            QString originalText = clipboard->text();
            emit getData(originalText.toLocal8Bit ());
        }
    }
    else if ((key >= Qt::Key_Space && key <= Qt::Key_ydiaeresis)
            && (modifier == Qt::NoModifier || modifier == Qt::ShiftModifier || modifier == Qt::KeypadModifier ))
    {
        if (m_localEchoEnabled && !m_displayHexValuesEnabled)
        {
            moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            QPlainTextEdit::keyPressEvent(e);
        }
        emit getData(e->text().toLocal8Bit());
    }
    else
    {
        key |= modifier;
        if (m_localEchoEnabled && !m_displayHexValuesEnabled)
        {
            if ((m_keyMap.contains(key) && m_keyMap[key].m_handleKey) || !m_keyMap.contains(key))
            {
                moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                QPlainTextEdit::keyPressEvent(e);
            }
        }
        if (m_keyMap.contains(key))
        {
            QByteArray data = m_keyMap[key].m_str.toLocal8Bit();
            if (data.length () > 0)
            {
                emit getData(data);
            }
        }
    }
}

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

/**
 * @brief Console::appendDataToConsole
 * Append serial data to console document.
 *
 * @param data Data to append.
 * @param scrollToEnd Scroll to end of document.
 * @param rebuild Rebuild whole text document.
 */
void Console::appendDataToConsole(const QByteArray &data, bool scrollToEnd, bool rebuild)
{
    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

    if (!m_displayHexValuesEnabled)
    {
        /* Normal mode */
        QByteArray data2;
        data2 = data;
        data2.replace (m_lineEndingRx, QByteArray("\n"));
        if (m_lineEndingRx == "\r\n" || m_lineEndingRx == "\n\r")
        {
            /* If '\r' remains in buffer, remove them. '\n' expected in next buffer */
            data2.replace (QString (m_lineEndingRx[0]), QByteArray());
        }
        foreach (char c, data2)
        {
            if (c == BACKSPACE)
            {
                /* Not good solution: backspace should not clear character
                 * only move cursor left.
                 */
//                moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.deletePreviousChar();
            }
            else
            {
                insertPlainText(QString(c));
            }
        }
    }
    else if (rebuild)
    {
        /* Hexadecimal display mode, rebuild console */
        insertPlainText (dumpBuf (data, m_hexWrap));
    }
    else
    {
        int size = m_data.size ();
        int mod = size % m_hexWrap;

        /* Hexadecimal display mode */
        if (mod > 0)
        {
            /* Delete last line, because it shall be rebuild again. */
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.deletePreviousChar();
            cursor.select(QTextCursor::LineUnderCursor);
            cursor.removeSelectedText();
            setTextCursor(cursor);
        }

        /* Get data which was in last line */
        QByteArray data2;
        data2 = m_data.right (mod);
        data2.append (data);

        insertPlainText (dumpBuf (data2, m_hexWrap));
    }

    if (scrollToEnd)
    {
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

/**
 * @brief Console::rebuildConsole
 * Regenerate console document.
 */
void Console::rebuildConsole()
{
    QPlainTextEdit::clear();
    appendDataToConsole (m_data, true, true);
}

/**
 * @brief Console::dumpBuf
 * Creates hexadecimal dump of a buffer.
 *
 * @param buf Buffer to convert hexadecimal ASCII.
 * @param hexWrap Wrap size. Usual values: 8, 16.
 * @return ASCII text.
 */
QString Console::dumpBuf(const QByteArray &buf, int hexWrap)
{
    QString str;
    QString str2;
    int i, j, s;
    int bufSize = buf.length ();

    /* Extend buffer size to be dividable with hexWrap */
    if (bufSize % hexWrap == 0)
    {
        s = bufSize;
    }
    else
    {
        s = bufSize + hexWrap - (bufSize % hexWrap);
    }
    for (i = 0; i < s; i++)
    {
        /* Print buffer in hexadecimal format */
        if (i < bufSize)
        {
            str2.sprintf ("%02X", buf[i]);
            str += str2;
        }
        else
        {
            str += "  ";
        }
        str += " ";
        if ((i + 1) % hexWrap == 0)
        {
            /* Print buffer in ASCII format */
            str += "  ";
            for (j = i - (hexWrap - 1); j <= i; j++)
            {
                if (j < bufSize)
                {
                    if (buf[j] >= 0x20 && buf[j] <= 0x7F)
                    {
                        str += buf[j];
                    }
                    else
                    {
                        str += ".";
                    }
                }
                else
                {
                    str += " ";
                }
            }
            str += "\n";
        }
    }

    return str;
}
