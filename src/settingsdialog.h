/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal

** Copyright (C) 2015-2024 Peter Ivanov <ivanovp@gmail.com>
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QAbstractButton>
#include <QTimer>
#include <QListWidgetItem>

#include "serialsettings.h"

/* 1: "Custom" text will appear at end of list of baud rate and serial port
 *    (unfinished! not working properly)
 * 0: combo box is editable (working, finished)
 */
#define USE_POLICY_CHECK    0

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0, SerialSettings *currentSerialSettings=NULL);
    ~SettingsDialog();

    SerialSettings::serialSettings_t serialSettings() const;

private slots:
    void showPortInfo(int idx);
    void on_buttonBox_clicked(QAbstractButton*);
#if USE_POLICY_CHECK
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
#endif
    void fillPortsInfo(bool forceUpdate=false);
    void on_profileListWidget_currentItemChanged(QListWidgetItem *item, QListWidgetItem *prevItem);
    void on_addButton_clicked();
    void on_removeButton_clicked();

    void on_autoRefreshSerialPortsCheckBox_stateChanged(int arg1);

    void on_autoRefreshSerialPortsSpinBox_valueChanged(int arg1);

    void on_filterSerialPortsLineEdit_editingFinished();

private:
    void fillPortsParameters();
    void ui2settings(SerialSettings::serialSettings_t *settings);
    void settings2ui(SerialSettings::serialSettings_t *settings);
    void settings2item(QListWidgetItem *item, SerialSettings::serialSettings_t *settings);
    void item2settings(QListWidgetItem *item, SerialSettings::serialSettings_t *settings);

private:
    Ui::SettingsDialog    * ui;
    SerialSettings        * m_currentSettings;
    QIntValidator         * m_intValidator;
    QTimer                  m_timer;
    QList<QSerialPortInfo>  m_availablePorts;

    enum
    {
        role_name = Qt::UserRole,
        role_baudRate,
        role_dataBits,
        role_parity,
        role_stringParity,
        role_stopBits,
        role_stringStopBits,
        role_flowControl,
        role_stringFlowControl
    };
};

#endif // SETTINGSDIALOG_H
