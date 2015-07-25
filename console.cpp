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

#include <QtCore/QDebug>
#include <QSettings>
#include <QMenu>

#define BACKSPACE 8
#define DELETE    127

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_dataSizeLimit(1 * 1024 * 1024) /* 1 MiB by default */
    , m_localEchoEnabled(false)
    , m_updateEnabled(true)
    , m_displayTimestampEnabled(false)
    , m_displayHexValuesEnabled(false)
    , m_hexValuePerLine(16)
    , m_lineEndingRx("\r\n")
    , m_lineEndingTx("\r")
{
//    setOverwriteMode(true);
    m_documentAscii = document ();
    m_documentHex = new QTextDocument (this);
    _ASSERT(m_documentHex);
    document()->setMaximumBlockCount(10000); // FIXME this should be configurable
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

    m_keyMap.insert(Qt::Key_Backspace, KeyMap(false, "\x08"));
    m_keyMap.insert(Qt::Key_Delete,    KeyMap(false, "\x7F"));
    m_keyMap.insert(Qt::Key_Return,    KeyMap(true, m_lineEndingTx));
    m_keyMap.insert(Qt::Key_Enter,     KeyMap(true, m_lineEndingTx));
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
//    QPlainTextEdit::clear ();
    m_documentAscii->clear ();
    m_documentHex->clear ();
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

void Console::keyPressEvent(QKeyEvent *e)
{
    if (e->key() >= Qt::Key_Space && e->key() <= Qt::Key_ydiaeresis)
    {
      if (m_localEchoEnabled)
      {
        moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        QPlainTextEdit::keyPressEvent(e);
      }
      emit getData(e->text().toLocal8Bit());
    }
    else
    {
      Qt::Key key = static_cast<Qt::Key> (e->key());
      if (m_localEchoEnabled)
      {
        if ((m_keyMap.contains(key) && m_keyMap[key].m_handleKey) || !m_keyMap.contains(key))
        {
          moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
          QPlainTextEdit::keyPressEvent(e);
        }
      }
      if (m_keyMap.contains(key))
      {
        emit getData(m_keyMap[key].m_str.toLocal8Bit());
      }
#if 0
      switch (e->key())
      {
        case Qt::Key_Backspace:
          if (m_localEchoEnabled)
          {
            QPlainTextEdit::keyPressEvent(e);
          }
          emit getData("\x08");
          break;
        case Qt::Key_Delete:
          if (m_localEchoEnabled)
          {
            QPlainTextEdit::keyPressEvent(e);
          }
          emit getData("\x7F");
          break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
          if (m_localEchoEnabled)
          {
            QPlainTextEdit::keyPressEvent(e);
          }
          qDebug() << __FUNCTION__ << "enter" << e->text().toLocal8Bit();
          emit getData(m_lineEndingTx.toLocal8Bit());
          break;
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
          QPlainTextEdit::keyPressEvent(e);
          break;
        default:
          QPlainTextEdit::keyPressEvent(e);
          break;
      }
#endif
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
//                moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.deletePreviousChar();
            }
            else
            {
                // TODO emulate keyPressEvent???
                //QKeyEvent event (QKeyEvent::KeyPress, c, Qt::NoModifier);
                //QPlainTextEdit::keyPressEvent(&event);
                //QKeyEvent event2 (QKeyEvent::KeyRelease, c, Qt::NoModifier);
                //QPlainTextEdit::keyPressEvent(&event2);
                insertPlainText(QString(c));
            }
        }
    }
    else
    {
        /* Hexadecimal display mode */
#if 0
        /* Delete last line, because it shall be rebuild again. */
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
//        cursor.deletePreviousChar(); // Added to trim the newline char when removing last line
        setTextCursor(cursor);
#endif
        int pos = m_data.size();
        foreach (char c, data)
        {
            QString s;
            s.sprintf("%02X ", c);
            insertPlainText(s);
            pos++;
            if ((pos % m_hexValuePerLine) == 0)
            {
                insertPlainText("\n");
            }
        }
    }

    if (scrollToEnd)
    {
        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

void Console::rebuildConsole()
{
    QPlainTextEdit::clear();
    appendDataToConsole (m_data, true);
}
