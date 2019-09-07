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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QDateTime>

/* 0: working, but should be improved */
/* 1: to be implemented */
#define CURSOR_MODE  0

class Console : public QPlainTextEdit
{
    Q_OBJECT

signals:
    void getData(const QByteArray &data);

public:
    explicit Console(QWidget *parent = 0);

    void putData(const QByteArray &dataRaw);

    bool isLocalEchoEnabled() const;
    void setLocalEchoEnabled(bool localEchoEnabled = true);

    bool isUpdateEnabled() const;
    void setUpdateEnabled(bool updateEnabled = true);

    int getDataSizeLimit() const;
    void setDataSizeLimit(int dataSizeLimit_bytes);

    int getDisplaySize() const;
    void setDisplaySize(int displaySize);

    int getHexWrap() const;
    void setHexWrap(int hexWrap);

    bool isDisplayTimestampEnabled() const;
    void setDisplayTimestampEnabled(bool displayTimestampEnabled = true);

    bool isDisplayHexValuesEnabled() const;
    void setDisplayHexValuesEnabled(bool displayHexValues = true);

    int getAutoWrapColumn() const;
    void setAutoWrapColumn(int autoWrapColumn);

    QString getLineEndingRx() const;
    void setLineEndingRx(const QString &lineEndingRx);

    QString getLineEndingTx() const;
    void setLineEndingTx(const QString &lineEndingTx);

    QByteArray getAllData() const;

    QString getTimestamp() const;
    void setTimestampFormatString(const QString& format);
    QString getTimestampFormatString();

public slots:
    void clear();
    void paste();

public:
    QVariant m_bgcolordef;
    QVariant m_inactbgcolordef;
    QVariant m_stoppedbgcolordef;
    QVariant m_fgcolordef;
    QVariant m_timestampcolordef;

private:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);
    void appendDataToConsole(const QByteArray &data, const QByteArray &dataRaw, bool scrollToEnd = true, bool rebuild = false);
    void rebuildConsole();
    QString dumpBuf(const QByteArray& buf, int hexWrap);
    void addTimestamp(QByteArray& buf);

    class KeyMap
    {
    public:
      KeyMap() : m_handleKey(false), m_str("") {}
      KeyMap(bool key, QString str) { m_handleKey = key; m_str = str; }

      bool m_handleKey; /**< true: Handle key by QPlainTextEdit */
      QString m_str;    /**< Text to be sent, when key is pressed */
    };

    bool m_localEchoEnabled;
    bool m_updateEnabled;
    bool m_displayTimestampEnabled;
    bool m_displayHexValuesEnabled;
    int m_hexWrap;
    QString m_lineEndingRx;
    QString m_lineEndingTx;
    QMap<unsigned int,KeyMap> m_keyMap;
    /* TODO There should be different documents (QDocument?) for
     * - ASCII without timestamp
     * - ASCII with timestamp
     * - hexadecimal
     * Adding colors to timestamp could possible...
     */
    QByteArray m_data;          /**< Serial data for ASCII view */
    QByteArray m_dataRaw;       /**< Raw serial data for hexadecimal view */
    QByteArray m_dataTimestamp; /**< Serial data with timestamp */
    int m_dataSizeLimit_bytes;
    int m_dataSizeHysteresis_percent;
    int m_autoWrapColumn;       /**< Automatically wrap text after m_autoWrapColumn characters */
    int m_noLineEndingCntr;     /**< Distance from last line ending character (for auto wrap) */
    QString m_timestampFormatString;
#if CURSOR_MODE == 1
    QCursor m_cursor;
#endif
};

#endif // CONSOLE_H
