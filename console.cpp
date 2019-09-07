/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
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
#include <QDateTime>

#include <QtCore/QDebug>
#include <QSettings>
#include <QMenu>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_localEchoEnabled(false)
    , m_updateEnabled(true)
    , m_displayTimestampEnabled(false)
    , m_displayHexValuesEnabled(false)
    , m_hexWrap(16)
    , m_lineEndingRx("\r\n")
    , m_lineEndingTx("\r")
    , m_dataSizeLimit_bytes(1 * 1024 * 1024) /* 1 MiB by default */
    , m_dataSizeHysteresis_percent(10) /* 10 % by default */
    , m_autoWrapColumn(80)  /* automatically wrap text after 80 characters */
    , m_noLineEndingCntr(0)
    , m_timestampFormatString("HH:mm:ss.zzz  ")
{
#if CURSOR_MODE == 1
    setOverwriteMode(true);
#endif
    setLineWrapMode(NoWrap);
    setAcceptDrops(false);
    setUndoRedoEnabled(false);
    document()->setMaximumBlockCount(10000);
    QSettings settings;
    QString fontStr = settings.value("console/font", "Monospace,12").toString();
    QFont font;
    font.fromString(fontStr);
    font.setStyleHint(QFont::TypeWriter);
    document()->setDefaultFont(font);
    QPalette p = palette();
    m_bgcolordef = QColor (Qt::black).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    m_inactbgcolordef = QColor (Qt::gray).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    m_stoppedbgcolordef = QColor (Qt::darkRed).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    m_fgcolordef = QColor (Qt::lightGray).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    m_timestampcolordef = QColor (Qt::darkGray).name(QColor::HexArgb); // Convert default color to "#aarrggbb" format string
    QColor inactbgcolor = settings.value("console/inactbgcolor", m_inactbgcolordef).toString();
    QColor fgcolor = settings.value("console/fgcolor", m_fgcolordef).toString();
    QColor timestampcolor = settings.value("console/timestampcolor", m_timestampcolordef).toString();
//    qDebug() << "bgcolor" << bgcolor;
//    qDebug() << "fgcolor" << fgcolor;
    p.setColor(QPalette::Base, inactbgcolor);
    p.setColor(QPalette::Text, fgcolor);
    p.setColor(QPalette::Dark, timestampcolor);
    setPalette(p);

    m_keyMap.clear();
    /* Backspace and delete are not handled by QPlainTextEdit */
    m_keyMap.insert(Qt::Key_Backspace,                      KeyMap(false, "\x08"));
    m_keyMap.insert(Qt::Key_Delete,                         KeyMap(false, "\x7F"));
    /* Others are handled by QPlainTextEdit */
    m_keyMap.insert(Qt::Key_Return,                         KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter,                          KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter | Qt::KeypadModifier,     KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_C | Qt::ControlModifier,        KeyMap(true, ""));
    m_keyMap.insert(Qt::Key_V | Qt::ControlModifier,        KeyMap(true, ""));
    m_keyMap.insert(Qt::Key_A | Qt::ControlModifier,        KeyMap(true, ""));
}

void Console::putData(const QByteArray &dataRaw)
{
    int i;
    int j;
    bool found;
    int length = m_lineEndingRx.length();
    QByteArray data;

    if (m_autoWrapColumn > 0)
    {
        for (i = 0; i < dataRaw.length(); i++)
        {
            /* Check if any of the line ending chars found in the buffer */
            found = false;
            for (j = 0; j < length; j++)
            {
                if (dataRaw[i] == m_lineEndingRx[j])
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                m_noLineEndingCntr = 0u;
            }
            else
            {
                m_noLineEndingCntr++;
            }
            if (m_noLineEndingCntr > m_autoWrapColumn)
            {
                /* No line ending found, wrap the line! */
                data += m_lineEndingRx;
                m_noLineEndingCntr = 0u;
            }
            data += dataRaw[i];
        }
    }
    else
    {
        /* Auto wrap disabled, copy buffer */
        data = dataRaw;
    }

    if (m_updateEnabled)
    {
        QScrollBar *bar = verticalScrollBar();
        /* Check if slider is scrolled to down */
        bool scrollToEnd = bar->sliderPosition() == bar->maximum();

        appendDataToConsole (data, dataRaw, scrollToEnd);
    }

    m_data.append(data);
    qDebug() << "m_data.len" << m_data.length();
    if (m_data.length () > m_dataSizeLimit_bytes)
    {
        /* Remove unwanted bytes */
        /* / 10 ---> 10 percent histeresys TODO configurable histeresys? */
        m_data.remove (0, m_data.length () - m_dataSizeLimit_bytes + m_dataSizeLimit_bytes / m_dataSizeHysteresis_percent);
    }

    m_dataRaw.append(dataRaw);
    qDebug() << "m_dataRaw.len" << m_dataRaw.length();
    if (m_dataRaw.length () > m_dataSizeLimit_bytes)
    {
        /* Remove unwanted bytes */
        /* / 10 ---> 10 percent histeresys TODO configurable histeresys? */
        m_dataRaw.remove (0, m_dataRaw.length () - m_dataSizeLimit_bytes + m_dataSizeLimit_bytes / m_dataSizeHysteresis_percent);
    }

    QByteArray data2 = data;
    addTimestamp(data2);
    m_dataTimestamp.append(data2);
    if (m_dataTimestamp.length () > m_dataSizeLimit_bytes)
    {
        /* Remove unwanted bytes */
        /* / 10 ---> 10 percent histeresys TODO configurable histeresys? */
        m_dataTimestamp.remove (0, m_dataTimestamp.length () - m_dataSizeLimit_bytes + m_dataSizeLimit_bytes / m_dataSizeHysteresis_percent);
    }
}

void Console::clear()
{
    qDebug() << __PRETTY_FUNCTION__;
    QPlainTextEdit::clear ();
    m_data.clear ();
    m_dataTimestamp.clear ();
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

QByteArray Console::getAllData() const
{
    if (m_displayTimestampEnabled)
    {
        return m_dataTimestamp;
    }
    else
    {
        return m_data;
    }
}

QString Console::getTimestamp() const
{
    QDateTime now = QDateTime::currentDateTime();
    return now.toString(m_timestampFormatString);
}

void Console::setTimestampFormatString(const QString &format)
{
    m_timestampFormatString = format;
}

QString Console::getTimestampFormatString()
{
    return m_timestampFormatString;
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

int Console::getAutoWrapColumn() const
{
    return m_autoWrapColumn;
}

void Console::setAutoWrapColumn(int autoWrapColumn)
{
    m_autoWrapColumn = autoWrapColumn;
}

bool Console::isDisplayTimestampEnabled() const
{
    return m_displayTimestampEnabled;
}

void Console::setDisplayTimestampEnabled(bool displayTimestampEnabled)
{
    if (m_displayTimestampEnabled != displayTimestampEnabled)
    {
        m_displayTimestampEnabled = displayTimestampEnabled;
        rebuildConsole();
    }
}

int Console::getDataSizeLimit() const
{
    return m_dataSizeLimit_bytes;
}

void Console::setDataSizeLimit(int dataSizeLimit_bytes)
{
    m_dataSizeLimit_bytes = dataSizeLimit_bytes;
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
 * @param data    Data to append (for ASCII view).
 * @param rawData Data to append (for hexadecimal view).
 * @param scrollToEnd Scroll to end of document.
 * @param rebuild Rebuild whole text document.
 */
void Console::appendDataToConsole(const QByteArray &data, const QByteArray &dataRaw, bool scrollToEnd, bool rebuild)
{
#if CURSOR_MODE == 0
    moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
#else
    setCursor(m_cursor);
#endif

    if (!m_displayHexValuesEnabled)
    {
        /* Normal mode */
        QByteArray data2, newLine;
        data2 = data;

        if (m_displayTimestampEnabled && !rebuild)
        {
            addTimestamp(data2);
        }

        newLine = QByteArray(NATIVE_LINEENDNG);
        data2.replace (m_lineEndingRx, newLine);
        if (m_lineEndingRx == "\r\n" || m_lineEndingRx == "\n\r")
        {
            /* If '\r' remains in buffer, remove them. '\n' expected in next buffer */
            data2.replace (QString ('\r'), QByteArray());
        }
        if (data2.contains (BACKSPACE))
        {
            while (data2.length () > 0)
            {
                int pos = data2.indexOf(BACKSPACE);
                if (pos >= 0)
                {
                    insertPlainText(QString(data2.left(pos)));
                    data2.remove(0, pos + 1);
#if CURSOR_MODE == 0
                    /* Not good solution: backspace should not clear character
                     * only move cursor left.
                     */
                    QTextCursor cursor = textCursor();
                    cursor.movePosition(QTextCursor::End);
                    cursor.deletePreviousChar();
#else
                    moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
#endif
                }
                else
                {
                    insertPlainText(QString(data2));
                    break;
                }
            }
        }
        else
        {
            insertPlainText(QString(data2));
        }
    }
    else if (rebuild)
    {
        /* Hexadecimal display mode, rebuild console */
        insertPlainText (dumpBuf (dataRaw, m_hexWrap));
    }
    else
    {
        int size = m_dataRaw.size ();
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
        data2 = m_dataRaw.right (mod);
        data2.append (dataRaw);

        insertPlainText (dumpBuf (data2, m_hexWrap));
    }
#if CURSOR_MODE == 1
    m_cursor = cursor();
#endif

    if (scrollToEnd)
    {
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
        /* Maybe only this needed */
        ensureCursorVisible();
    }
}

/**
 * @brief Console::rebuildConsole
 * Regenerate console document.
 */
void Console::rebuildConsole()
{
    QPlainTextEdit::clear();
    if (!m_displayHexValuesEnabled && m_displayTimestampEnabled)
    {
        appendDataToConsole (m_dataTimestamp, m_dataRaw, true, true);
    }
    else
    {
        appendDataToConsole (m_data, m_dataRaw, true, true);
    }
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
            str2.sprintf ("%02X", (quint8) buf[i]);
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
                    if ((quint8) buf[j] >= 0x20u && (quint8)buf[j] <= 0x7Fu)
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

void Console::addTimestamp(QByteArray &buf)
{
    QString timestamp = getTimestamp();

    /* Adding timestamps after every new line character */
    QString data2 = buf;
    for (int i = data2.length() - 1; i >= 0; i--)
    {
        if (data2[i] == m_lineEndingRx.right(1).toLatin1())
        {
            data2.insert(i + 1, timestamp.toLocal8Bit());
        }
    }

    buf = data2.toLocal8Bit();
}
