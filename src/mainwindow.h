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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QPushButton>
#include <QtSerialPort/QSerialPort>
#include <QProgressBar>
#include <QTimer>

#include "multistring.h"
#include "multivalidator.h"
#include "serialsettings.h"
#include "settingsdialog.h"
#include "common.h"

#if WINDOWS
#define USE_UPDATE_TIMER    1
#else
#define USE_UPDATE_TIMER    0
#endif

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class SerialThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  enum
  {
    UserData_ButtonId = Qt::UserRole
  };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setEnableConsole(bool enable);
    void setEnableCustomText(int idx, bool enable = true);
    void setVisibleCustomText(int idx, bool visible = true, const QString &text = "");
private slots:
    void openSerialPort();
    void closeSerialPort(const QString& errorMsg = "");
    void updateBackgroundColor(void);
    void about();
    void writeData(const QByteArray &data);
    void readData();

    void serialMessage(QString message, bool error);
    void serialPortStatusChanged(bool opened);
    void handleError(QSerialPort::SerialPortError error);

    void on_actionLocal_echo_triggered(bool checked);
    void on_actionSet_font_triggered();
    void on_actionSet_background_color_triggered();
    void on_actionSet_foreground_color_triggered();
    void onSendLineEdit_returnPressed();
    void on_actionStop_update_triggered(bool checked);
    void on_sendButton_clicked();
    void on_actionViewSendInput_triggered(bool checked);
    void on_actionHexadecimal_view_triggered(bool checked);
    void on_actionConfigure_console_triggered();
    void on_actionShow_line_status_triggered(bool checked);

    void serialProgress(QString message, int percent);
    void serialFinish();
    void serialPinoutsChanged(QSerialPort::PinoutSignals pinoutSignals);

    void on_actionSend_file_triggered();
    void on_actionSave_file_triggered();
    void on_actionToggle_DTR_triggered();
    void on_actionToggle_RTS_triggered();
    void on_actionSend_custom_text_1_triggered();
    void on_actionSend_custom_text_2_triggered();
    void on_actionSend_custom_text_3_triggered();
    void on_actionSend_custom_text_4_triggered();
    void on_actionSend_custom_text_5_triggered();
    void on_actionSend_custom_text_6_triggered();

    void onSendModeComboBox_currentIndexChanged();
    QStringList getCurrentHistory();
    QStringList loadHistory(Multistring::mode_t mode);
    void saveHistory(Multistring::mode_t mode, const QStringList& history);

    void on_actionSet_inactive_background_color_triggered();
    void on_actionSet_stopped_background_color_triggered();
    void on_actionConfigure_triggered();

    void on_actionShow_timestamp_triggered(bool checked);

    void on_actionSet_timestamp_color_triggered();

    void find(QString searchStr = "", bool caseSens = false, bool wholeWords = false, bool regEx = false);
    void on_actionFind_triggered();
    void on_actionFind_next_triggered();

    void on_actionSelectProfile_triggered();

private:
    Ui::MainWindow *ui;
    Console *m_console;
    SerialSettings m_currentSerialSettings;
    SerialThread *m_serialThread;
    bool m_serialError;
    QProgressBar *m_progressBar;
    QPushButton *m_abortButton;
    QVector<QString> m_customTexts;
    QVector<bool> m_customTextsEnabled;
    Multistring m_sendLine;
    MultiValidator * m_multivalidator;
    QStringList m_sendLineHistories[4]; /**< History for following modes: ASCII, hexadecimal, decimal, binary */
#if USE_UPDATE_TIMER
    QTimer      m_updateTimer;          /**< Console update timer */
#endif
};

#endif // MAINWINDOW_H
