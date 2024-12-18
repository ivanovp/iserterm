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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "multivalidator.h"
#include "common.h"
#include "version.h"
#include "consolesettingsdialog.h"
#include "serialthread.h"
#include "multistring.h"
#include "multivalidator.h"
#include "shiftdeleventfilter.h"
#include "finddialog.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QFontDialog>
#include <QSettings>
#include <QColorDialog>
#include <QtSerialPort/QSerialPort>
#include <QProgressBar>
#include <QFlags>
#include <QFileDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCompleter>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialError(false)
{
    QSettings settings;

    // settings.setDefaultFormat(QSettings::Format::IniFormat);
    settings.setDefaultFormat(QSettings::Format::NativeFormat);
    ui->setupUi(this);

    m_currentSerialSettings.loadSettings();
    m_multivalidator = new MultiValidator (this);

    m_customTexts.reserve(CUSTOM_TEXT_NUM);
    m_customTextsEnabled.reserve(CUSTOM_TEXT_NUM);
    for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
    {
        QString text = settings.value(QString("serial/customText%1").arg(i + 1), "").toString();
        bool enabled = settings.value(QString("serial/customText%1Enabled").arg(i + 1), false).toBool();
        m_customTexts.append(text);
        m_customTextsEnabled.append(enabled);
        setEnableCustomText(i, false);
        setVisibleCustomText(i, enabled, text);
    }

    setWindowIcon (QIcon (":/images/iserterm.png"));
    setWindowTitle(VER_PRODUCTNAME_STR);
    resize(settings.value("window/width", 500).toInt(), settings.value("window/height", 300).toInt());
    m_console = new Console;
    setEnableConsole(false);
    ui->consoleWidget->addWidget(m_console);
    ui->sendModeComboBox->addItem(tr("ASCII"), QVariant(Multistring::ASCII));
    ui->sendModeComboBox->addItem(tr("Hex"), QVariant(Multistring::Hexadecimal));
    ui->sendModeComboBox->addItem(tr("Dec"), QVariant(Multistring::Decimal));
    ui->sendModeComboBox->addItem(tr("Bin"), QVariant(Multistring::Binary));
//    ui->sendButtonGroup->setId(ui->asciiRadioButton, Multistring::ASCII);
//    ui->sendButtonGroup->setId(ui->hexRadioButton, Multistring::Hexadecimal);
//    ui->sendButtonGroup->setId(ui->decRadioButton, Multistring::Decimal);
//    ui->sendButtonGroup->setId(ui->binRadioButton, Multistring::Binary);

    switch (settings.value("console/sendModeComboBox", 0).toInt())
    {
        default:
        case 0:
            ui->sendModeComboBox->setCurrentText(tr("ASCII"));
            break;

        case 1:
            ui->sendModeComboBox->setCurrentText(tr("Hex"));
            break;

        case 2:
            ui->sendModeComboBox->setCurrentText(tr("Dec"));
            break;

        case 3:
            ui->sendModeComboBox->setCurrentText(tr("Bin"));
            break;

    }
    Multistring::mode_t mode = static_cast<Multistring::mode_t> (ui->sendModeComboBox->currentData().toInt());
    m_multivalidator->setMode(mode);
    m_sendLine.setMode(mode);

    ui->sendLineEdit->setValidator(m_multivalidator);
    ui->sendLineEdit->setEditable(true);
    ui->sendLineEdit->setDuplicatesEnabled(false);
    ui->sendLineEdit->addItems(loadHistory(mode));
    ui->sendLineEdit->lineEdit()->setText(settings.value("console/sendLineEdit").toString());
    QObject * evfilter = new ShiftDelEventFilter;
    evfilter->setParent(ui->sendLineEdit);
    ui->sendLineEdit->installEventFilter(evfilter);
    ui->sendLineEdit->completer()->setCompletionMode(static_cast<QCompleter::CompletionMode>(settings.value("completion/mode").toInt()));
    ui->sendLineEdit->completer()->setCaseSensitivity(static_cast<Qt::CaseSensitivity>(settings.value("completion/caseSensitivity").toInt()));
    ui->sendLineEdit->view()->installEventFilter(evfilter);
    ui->eolCheckBox->setChecked(settings.value("console/eolCheckBox").toBool());
    m_console->setAutoWrapColumn (settings.value("serial/autoWrapColumn", m_console->getAutoWrapColumn()).toInt ());
    m_console->setLineEndingRx (settings.value("serial/lineEndingRx", m_console->getLineEndingRx ()).toString ());
    m_console->setLineEndingTx (settings.value("serial/lineEndingTx", m_console->getLineEndingTx ()).toString ());
    m_console->setDataSizeLimit (settings.value("serial/dataSizeLimit", m_console->getDataSizeLimit ()).toInt ());
    m_console->setDisplaySize (settings.value("serial/displaySize", m_console->getDisplaySize ()).toInt ());
    m_console->setHexWrap (settings.value("serial/hexWrap", m_console->getHexWrap ()).toInt ());
    m_console->setTimestampFormatString(settings.value("console/timestampFormatString", m_console->getTimestampFormatString()).toString());

    m_serialThread = new SerialThread(NULL, &m_currentSerialSettings);
    m_serialThread->setDelayAfterBytes_ms (settings.value ("serial/delayAfterBytes_ms", m_serialThread->getDelayAfterBytes_ms ()).toInt());
    m_serialThread->setDelayAfterChr_ms(settings.value ("serial/delayAfterNewline_ms", m_serialThread->getDelayAfterChr_ms()).toInt(),
                                        m_console->getLineEndingTx().right(1).toLatin1());
    m_serialThread->setLineEndingRx(m_console->getLineEndingRx());
    m_serialThread->setLineEndingTx(m_console->getLineEndingTx());
    m_serialThread->start (QThread::NormalPriority);
//    m_serialSettingsDialog = new SettingsDialog;

    ui->actionLocal_echo->setChecked(settings.value("serial/localEchoEnabled", true).toBool());
    bool showLineStatus = settings.value("console/showLineStatus", true).toBool();
    ui->actionShow_line_status->setChecked(showLineStatus);
    on_actionShow_line_status_triggered(showLineStatus);
    bool showTimestamp = settings.value("console/showTimestamp", false).toBool();
    ui->actionShow_timestamp->setChecked(showTimestamp);
    on_actionShow_timestamp_triggered(showTimestamp);
    bool viewSendInput = settings.value("console/viewSendInput", true).toBool();
    ui->actionViewSendInput->setChecked(viewSendInput);
    on_actionViewSendInput_triggered(viewSendInput);
    ui->actionQuit->setEnabled(true);
    m_progressBar = new QProgressBar(this);
    m_progressBar->hide();
    ui->statusBar->addPermanentWidget(m_progressBar);
    m_abortButton = new QPushButton(tr("Abort"), this);
    m_abortButton->hide();
    ui->statusBar->addPermanentWidget(m_abortButton);

    MY_ASSERT(connect(ui->sendLineEdit->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onSendLineEdit_returnPressed())));
    MY_ASSERT(connect(ui->sendModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSendModeComboBox_currentIndexChanged())));

    MY_ASSERT(connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort())));
    MY_ASSERT(connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort())));
    MY_ASSERT(connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close())));
//    MY_ASSERT(connect(ui->actionConfigure, SIGNAL(triggered()), m_serialSettingsDialog, SLOT(show())));
    MY_ASSERT(connect(ui->actionClear, SIGNAL(triggered()), m_console, SLOT(clear())));
    MY_ASSERT(connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about())));
    MY_ASSERT(connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt())));

    MY_ASSERT(connect(m_serialThread, SIGNAL(message(QString,bool)), this, SLOT(serialMessage(QString,bool))));
    MY_ASSERT(connect(m_serialThread, SIGNAL(portStatusChanged(bool)), this, SLOT(serialPortStatusChanged(bool))));
    MY_ASSERT(connect(m_serialThread, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError))));
#if USE_UPDATE_TIMER == 0
    MY_ASSERT(connect(m_serialThread, SIGNAL(readyRead()), this, SLOT(readData())));
#endif
    MY_ASSERT(connect(m_serialThread, SIGNAL(progress(QString,int)), this, SLOT(serialProgress(QString,int))));
    MY_ASSERT(connect(m_serialThread, SIGNAL(finish()), this, SLOT(serialFinish())));
    MY_ASSERT(connect(m_serialThread, SIGNAL(pinoutSignalsChanged(QSerialPort::PinoutSignals)),
                      this, SLOT(serialPinoutsChanged(QSerialPort::PinoutSignals))));

    MY_ASSERT(connect(m_console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray))));

    MY_ASSERT(connect(m_abortButton, SIGNAL(pressed()), m_serialThread, SLOT(abortSend())));

#if USE_UPDATE_TIMER
    /* Timer is used to prevent flooding of console with data */
    m_updateTimer.setSingleShot(false);
    m_updateTimer.setInterval(50); /* FIXME this could be configurable */
    m_updateTimer.start();

    MY_ASSERT(connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(readData())));
#endif
}

MainWindow::~MainWindow()
{
    QSettings settings;

    m_currentSerialSettings.saveSettings();

    settings.setValue("window/width", size().width());
    settings.setValue("window/height", size().height());
    settings.setValue("serial/localEchoEnabled", ui->actionLocal_echo->isChecked());
    settings.setValue("console/viewSendInput", ui->actionViewSendInput->isChecked());
    settings.setValue("console/showLineStatus", ui->actionShow_line_status->isChecked());
    settings.setValue("console/sendModeComboBox", ui->sendModeComboBox->currentIndex());
    settings.setValue("console/sendLineEdit", ui->sendLineEdit->lineEdit()->text());
    settings.setValue("console/eolCheckBox", ui->eolCheckBox->isChecked());
    settings.setValue("console/showTimestamp", ui->actionShow_timestamp->isChecked());
    saveHistory(m_sendLine.getMode(), getCurrentHistory());
    if (m_serialThread)
    {
        m_serialThread->stop(250);
    }
    delete m_serialThread;
    m_serialThread = NULL;
//    delete m_serialSettingsDialog;
//    m_serialSettingsDialog = NULL;
    delete ui;
    ui = NULL;
}

void MainWindow::setEnableConsole(bool enable)
{
    m_console->setReadOnly (!enable);
//    console->setEnabled(enable);
//    ui->sendLayout->setEnabled(enable);
    ui->sendLabel->setEnabled(enable);
    ui->sendLineEdit->setEnabled(enable);
    ui->sendModeComboBox->setEnabled(enable);
    ui->eolCheckBox->setEnabled(enable);
    ui->sendButton->setEnabled(enable);
    ui->actionConnect->setEnabled(!enable);
    ui->actionDisconnect->setEnabled(enable);
//    ui->actionConfigure->setEnabled(!enable);
    ui->actionToggle_DTR->setEnabled(enable);
    ui->actionToggle_RTS->setEnabled(enable);

    for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
    {
        setVisibleCustomText(i, m_customTextsEnabled[i], m_customTexts[i]);
        setEnableCustomText(i, enable);
    }

    if (!enable)
    {
        ui->rxLabel->setEnabled(false);
        ui->txLabel->setEnabled(false);
        ui->dtrLabel->setEnabled(false);
        ui->dcdLabel->setEnabled(false);
        ui->dsrLabel->setEnabled(false);
        ui->riLabel->setEnabled(false);
        ui->rtsLabel->setEnabled(false);
        ui->ctsLabel->setEnabled(false);
        ui->brkLabel->setEnabled(false);
    }
}

void MainWindow::setEnableCustomText(int idx, bool enable)
{
    switch (idx)
    {
        case 0:
            ui->actionSend_custom_text_1->setEnabled(enable);
            break;
        case 1:
            ui->actionSend_custom_text_2->setEnabled(enable);
            break;
        case 2:
            ui->actionSend_custom_text_3->setEnabled(enable);
            break;
        case 3:
            ui->actionSend_custom_text_4->setEnabled(enable);
            break;
        case 4:
            ui->actionSend_custom_text_5->setEnabled(enable);
            break;
        case 5:
            ui->actionSend_custom_text_6->setEnabled(enable);
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid text index";
            Q_ASSERT(0);
            break;
    }
}

void MainWindow::setVisibleCustomText(int idx, bool visible, const QString& text)
{
    QAction* action = NULL;
    switch (idx)
    {
        case 0:
            action = ui->actionSend_custom_text_1;
            break;
        case 1:
            action = ui->actionSend_custom_text_2;
            break;
        case 2:
            action = ui->actionSend_custom_text_3;
            break;
        case 3:
            action = ui->actionSend_custom_text_4;
            break;
        case 4:
            action = ui->actionSend_custom_text_5;
            break;
        case 5:
            action = ui->actionSend_custom_text_6;
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid text index";
            Q_ASSERT(0);
            break;
    }
    if (action)
    {
        action->setVisible(visible);
        if (text.length())
        {
            QString s = QString(tr("Send custom text %1:\n%2")).arg(idx).arg(text);
            action->setToolTip(s);
        }
    }
}

void MainWindow::openSerialPort()
{
    qDebug() << __FUNCTION__ << m_currentSerialSettings.toString();
    // It is not necessary to set port name, baud rate in serial thread as m_currentSerialSettings
    // already contains the settings
    m_serialThread->open(QIODevice::ReadWrite);
    /* serialPortOpened() will triggered if opened successfully */
    /* serialMessage() will triggered if error occured */
}

void MainWindow::closeSerialPort(const QString &errorMsg)
{
    Q_UNUSED(errorMsg);

    if (m_serialThread->isOpen())
    {
        qDebug() << __FUNCTION__ << m_currentSerialSettings.toString();
        m_serialThread->close();
    }
    else
    {
        qDebug() << __FUNCTION__ << "not opened!";
    }
}

void MainWindow::updateBackgroundColor()
{
    QSettings settings;
    QPalette palette = m_console->palette();

    if (m_serialThread->isOpen())
    {
        /* Port opened */
        if (m_console->isUpdateEnabled())
        {
            /* Normal mode */
            QColor color = settings.value("console/bgcolor", m_console->m_bgcolordef).toString();
            palette.setColor(QPalette::Base, color);
        }
        else
        {
            /* STOP button pressed */
            QColor color = settings.value("console/stoppedbgcolor", m_console->m_stoppedbgcolordef).toString();
            palette.setColor(QPalette::Base, color);
        }
    }
    else
    {
        /* Port closed */
        QColor color = settings.value("console/inactbgcolor", m_console->m_inactbgcolordef).toString();
        palette.setColor(QPalette::Base, color);
    }
    m_console->setPalette(palette);
}

void MainWindow::about()
{
    QString aboutStr;
    aboutStr = QString(tr("%1 v%2.%3.%4\n"
                          "RS-232 serial terminal software based on Simple Terminal example.\n"
                          "License: GPLv3.\n"
                          "Compiled on " __DATE__ " " __TIME__ ".\n"
                          "Git version %5.\n"
                          "Copyright (C) Peter Ivanov <ivanovp@gmail.com>, 2015-2024\n"
                          "\n"
                          "Simple Terminal authors:\n"
                          "Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>\n"
                          "Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>\n"
                          "\n"
                          "This program is free software: you can redistribute it and/or modify "
                          "it under the terms of the GNU General Public License as published by "
                          "the Free Software Foundation either version 3 of the License or "
                          "(at your option) any later version.\n"
                          "\n"
                          "This program is distributed in the hope that it will be useful "
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                          "GNU General Public License for more details.\n"
                          "\n"
                          "You should have received a copy of the GNU General Public License "
                          "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n"
                          )).arg(VER_PRODUCTNAME_STR).arg(VER_PRODUCT_MAJOR)
                   .arg(VER_PRODUCT_MINOR).arg(VER_PRODUCT_RELEASE).arg(GIT_VERSION);
    QMessageBox::about(this, tr("About"), aboutStr);
}

void MainWindow::writeData(const QByteArray &data)
{
    /* Send user input */
    m_serialThread->write (data);
}

void MainWindow::readData()
{
    QByteArray data = m_serialThread->readAll();
    if (data.length())
    {
        /* Receive serial data and show on console */
        m_console->putData(data);
    }
}

void MainWindow::serialMessage(QString message, bool error)
{
    if (error)
    {
        QMessageBox::critical(this, tr("Error"), message);
        ui->statusBar->showMessage(tr("Error: ") + message);
    }
    else
    {
        ui->statusBar->showMessage(message);
    }
    updateBackgroundColor();
}

void MainWindow::serialPortStatusChanged(bool opened)
{
    setEnableConsole(opened);
    if (opened)
    {
        //console->setLocalEchoEnabled(p.localEchoEnabled);
        m_console->setLocalEchoEnabled(ui->actionLocal_echo->isChecked());
        ui->statusBar->showMessage(tr("Connected to %1: %2, %3%4%5, %6")
                                   .arg(m_serialThread->portName()).arg(m_serialThread->baudRate()).arg(m_serialThread->dataBits())
                                   .arg(m_serialThread->parityStr()[0]).arg(m_serialThread->stopBits()).arg(m_serialThread->flowControlStr()));
        QString title = QString("%1 - %2, %3, %4%5%6, %7").arg(VER_PRODUCTNAME_STR).arg(m_serialThread->portName())
                .arg(m_serialThread->baudRate()).arg(m_serialThread->dataBits()).arg(m_serialThread->parityStr()[0]).arg(m_serialThread->stopBits())
                .arg(m_serialThread->flowControlStr());
        setWindowTitle(title);
    }
    else
    {
        if (m_serialError)
        {
            QString errorMsg = m_serialThread->errorString();
            ui->statusBar->showMessage(tr("Disconnected: %1").arg(errorMsg));
        }
        else
        {
            ui->statusBar->showMessage(tr("Disconnected"));
        }
        m_serialError = false;
        setWindowTitle(VER_PRODUCTNAME_STR);
    }
    updateBackgroundColor();
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError)
    {
        m_serialError = true;
        m_serialThread->close();
    }
}

void MainWindow::on_actionLocal_echo_triggered(bool checked)
{
    m_console->setLocalEchoEnabled(checked);
}

void MainWindow::on_actionSet_font_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_console->document()->defaultFont());

    if (ok)
    {
        m_console->document()->setDefaultFont(font);
        QSettings settings;
        settings.setValue("console/font", font.toString());
    }
}

void MainWindow::on_actionSet_background_color_triggered()
{
    QSettings settings;
    QPalette palette = m_console->palette();
    QColor colorOriginal = settings.value("console/bgcolor", m_console->m_bgcolordef).toString();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose background color");
    if (color.isValid())
    {
        settings.setValue("console/bgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Base, color);
        updateBackgroundColor();
    }
}


void MainWindow::on_actionSet_inactive_background_color_triggered()
{
    QSettings settings;
    QPalette palette = m_console->palette();
    QColor colorOriginal = settings.value("console/inactbgcolor", m_console->m_inactbgcolordef).toString();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose background color for inactive state");
    if (color.isValid())
    {
        settings.setValue("console/inactbgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Base, color);
        updateBackgroundColor();
    }
}

void MainWindow::on_actionSet_stopped_background_color_triggered()
{
    QSettings settings;
    QPalette palette = m_console->palette();
    QColor colorOriginal = settings.value("console/stoppedbgcolor", m_console->m_stoppedbgcolordef).toString();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose background color for stopped state");
    if (color.isValid())
    {
        settings.setValue("console/stoppedbgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Base, color);
        updateBackgroundColor();
    }
}

void MainWindow::on_actionSet_foreground_color_triggered()
{
    QPalette palette = m_console->palette();
    QColor colorOriginal = palette.color(QPalette::Text).toRgb();
    QColor color;
//    qDebug() << "fgcolor orig" << colorOriginal;
    color = QColorDialog::getColor(colorOriginal, this, "Choose foreground color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/fgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Text, color);
        m_console->setPalette(palette);
    }
}

void MainWindow::onSendLineEdit_returnPressed()
{
    QString str = ui->sendLineEdit->lineEdit()->text();
    QByteArray data;

    /* Check if text was added to history, neccessary when Send button was
     * used! */
    if (ui->sendLineEdit->findText(str) < 0)
    {
        /* Text not found in history, add it! */
        ui->sendLineEdit->addItem(str);
    }
    m_sendLine.setString(str);
    data = m_sendLine.getByteArray();
    if (ui->eolCheckBox->isChecked())
    {
        data.append(m_console->getLineEndingTx().toLocal8Bit());
    }
    if (data.length())
    {
        m_serialThread->write(data);
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(data);
        }
    }

    ui->sendLineEdit->lineEdit()->selectAll();
}

void MainWindow::on_actionStop_update_triggered(bool checked)
{
    m_console->setUpdateEnabled(!checked);
    updateBackgroundColor();
}

void MainWindow::on_sendButton_clicked()
{
    onSendLineEdit_returnPressed();
}

void MainWindow::on_actionViewSendInput_triggered(bool checked)
{
    ui->sendLabel->setVisible(checked);
    ui->sendLineEdit->setVisible(checked);
    ui->sendModeComboBox->setVisible(checked);
    ui->eolCheckBox->setVisible(checked);
    ui->sendButton->setVisible(checked);
}

void MainWindow::on_actionHexadecimal_view_triggered(bool checked)
{
    m_console->setDisplayHexValuesEnabled (checked);
}

void MainWindow::on_actionConfigure_console_triggered()
{
    ConsoleSettingsDialog *dialog = new ConsoleSettingsDialog(this);
    QSettings settings;

    // Tab1
    dialog->setAutoWrapColumn(m_console->getAutoWrapColumn());
    dialog->setLineEndingRx(m_console->getLineEndingRx ());
    dialog->setLineEndingTx(m_console->getLineEndingTx ());
    dialog->setDataBufferSize(m_console->getDataSizeLimit () >> 20);
    dialog->setDisplaySize(m_console->getDisplaySize ());
    dialog->setHexWrap(m_console->getHexWrap ());
    dialog->setDelayAfterSendByte(m_serialThread->getDelayAfterBytes_ms());
    dialog->setDelayAfterSendNewLine(m_serialThread->getDelayAfterChr_ms());
    dialog->setTimestampFormatString(m_console->getTimestampFormatString());
    dialog->setAutoLogFileName(m_serialThread->autoLogFileName());
    dialog->setAutoLogFilePath(m_serialThread->autoLogFilePath());
    dialog->setAutoLogTimestampFormatString(m_serialThread->getTimestampFormatString());
    // Tab2
//    dialog->setCompletionMode(settings.value("completion/mode").toInt());
//    dialog->setCompletionCaseSensitivity(settings.value("completion/caseSensitivity").toInt());
    dialog->setCompletionMode(ui->sendLineEdit->completer()->completionMode());
    dialog->setCompletionCaseSensitivity(ui->sendLineEdit->completer()->caseSensitivity());
    // Tab3
    for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
    {
        QString text = settings.value(QString("serial/customText%1").arg(i + 1), "").toString();
        bool enabled = settings.value(QString("serial/customText%1Enabled").arg(i + 1), false).toBool();
        dialog->setCustomText(i, text);
        dialog->setCustomTextEnabled(i, enabled);
    }

    int result = dialog->exec();
//    qDebug() << __PRETTY_FUNCTION__ << result;

    if (result == ConsoleSettingsDialog::Accepted)
    {
        int autoWrapColumn = dialog->getAutoWrapColumn();
        QString lineEndingRx = dialog->getLineEndingRx();
        QString lineEndingTx = dialog->getLineEndingTx();
        int dataSizeLimit = dialog->getDataBufferSize() << 20;
        int displaySize = dialog->getDisplaySize();
        int hexWrap = dialog->getHexWrap();
        int delayAfterBytes_ms = dialog->getDelayAfterSendByte();
        int delayAfterNewline_ms = dialog->getDelayAfterSendNewLine();
        QString timestampFormatString = dialog->getTimestampFormatString();
        QSettings settings;
        for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
        {
            m_customTexts[i] = dialog->getCustomText(i);
            m_customTextsEnabled[i] = dialog->isCustomTextEnabled(i);
            setVisibleCustomText(i, m_customTextsEnabled[i], m_customTexts[i]);
        }
        m_console->setAutoWrapColumn(autoWrapColumn);
        m_console->setLineEndingRx(lineEndingRx);
        m_console->setLineEndingTx(lineEndingTx);
        m_console->setDataSizeLimit(dataSizeLimit);
        m_console->setDisplaySize (displaySize);
        m_console->setHexWrap (hexWrap);
        m_console->setTimestampFormatString(timestampFormatString);
        m_serialThread->setDelayAfterBytes_ms(delayAfterBytes_ms);
        m_serialThread->setDelayAfterChr_ms(delayAfterNewline_ms, lineEndingTx.right(1).toLatin1());
        m_serialThread->setLineEndingRx(lineEndingRx);
        m_serialThread->setLineEndingTx(lineEndingTx);
        m_serialThread->enableAutoLog(dialog->isAutoLogEnabled());
        m_serialThread->setAutoLogFileName(dialog->getAutoLogFileName());
        m_serialThread->setAutoLogFilePath(dialog->getAutoLogFilePath());
        m_serialThread->setTimestampFormatString(dialog->getAutoLogTimestampFormatString());
        ui->sendLineEdit->completer()->setCompletionMode(dialog->getCompletionMode());
        ui->sendLineEdit->completer()->setCaseSensitivity(dialog->getCompletionCaseSensitivity());
    }

    delete dialog;
}

void MainWindow::on_actionShow_line_status_triggered(bool checked)
{
    ui->line->setHidden(!checked);
    ui->rxLabel->setHidden(!checked);
    ui->txLabel->setHidden(!checked);
    ui->dsrLabel->setHidden(!checked);
    ui->ctsLabel->setHidden(!checked);
    ui->dcdLabel->setHidden(!checked);
    ui->riLabel->setHidden(!checked);
    ui->dtrLabel->setHidden(!checked);
    ui->rtsLabel->setHidden(!checked);
    ui->brkLabel->setHidden(!checked);
}

void MainWindow::serialProgress(QString message, int percent)
{
    if (message.length())
    {
        ui->statusBar->showMessage(message);
    }
    m_progressBar->show();
    m_progressBar->setValue(percent);
    m_abortButton->show();
}

void MainWindow::serialFinish()
{
    m_progressBar->hide();
    m_abortButton->hide();
}

void MainWindow::serialPinoutsChanged(QSerialPort::PinoutSignals pinoutSignals)
{
    ui->rxLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::TransmittedDataSignal));
    ui->txLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::ReceivedDataSignal));
    ui->dtrLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::DataTerminalReadySignal));
    ui->dcdLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::DataCarrierDetectSignal));
    ui->dsrLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::DataSetReadySignal));
    ui->riLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::RingIndicatorSignal));
    ui->rtsLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::RequestToSendSignal));
    ui->ctsLabel->setEnabled(pinoutSignals.testFlag(QSerialPort::ClearToSendSignal));
}

void MainWindow::on_actionSend_file_triggered()
{
    if (m_serialThread->isOpen())
    {
        QSettings settings;
        QString dir = settings.value ("console/sendFileDir").toString();
        QString fileName = QFileDialog::getOpenFileName(this, tr("Send file"), dir, tr("Text files (*.log *.txt);;All files (*.*)"));
        if (fileName.length())
        {
            QFileInfo fileInfo (fileName);
            dir = fileInfo.absolutePath();
            settings.setValue("console/sendFileDir", dir);
            QFile file (fileName);

            if (file.open(QFile::ReadOnly))
            {
                QByteArray data = file.readAll();
                if (data.length() > 0)
                {
                    m_serialThread->write(data);
                }
                else
                {
                    QMessageBox::critical(this, tr("Cannot read file"), file.errorString());
                }
                file.close();
            }
            else
            {
                QMessageBox::critical(this, tr("Cannot open file for reading"), file.errorString());
            }
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Cannot send file"), tr("Cannot send file if serial port is not opened."));
    }
}

void MainWindow::on_actionSave_file_triggered()
{
    QSettings settings;
    QString dir = settings.value ("console/saveDir").toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save raw serial data"), dir, tr("Text files (*.log *.txt);;All files (*.*)"));
    if (fileName.length())
    {
        QFileInfo fileInfo (fileName);
        dir = fileInfo.absolutePath();
        settings.setValue("console/saveDir", dir);
        QFile file (fileName);

        if (file.open(QFile::WriteOnly))
        {
            QByteArray data = m_console->getAllData();
            if (file.write(data) != data.length())
            {
                QMessageBox::critical(this, tr("Cannot write file"), file.errorString());
            }
            file.close();
        }
        else
        {
            QMessageBox::critical(this, tr("Cannot open file for writing"), file.errorString());
        }
    }
}

void MainWindow::on_actionToggle_DTR_triggered()
{
    if (m_serialThread->flowControl() != QSerialPort::HardwareControl)
    {
        m_serialThread->setDataTerminalReady(!m_serialThread->isDataTerminalReady());
    }
}

void MainWindow::on_actionToggle_RTS_triggered()
{
    if (m_serialThread->flowControl() != QSerialPort::HardwareControl)
    {
        m_serialThread->setRequestToSend(!m_serialThread->isRequestToSend());
    }
}

void MainWindow::on_actionSend_custom_text_1_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText1Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText1", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::on_actionSend_custom_text_2_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText2Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText2", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::on_actionSend_custom_text_3_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText3Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText3", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::on_actionSend_custom_text_4_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText4Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText4", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::on_actionSend_custom_text_5_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText5Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText5", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::on_actionSend_custom_text_6_triggered()
{
    QSettings settings;
    bool enabled = settings.value("serial/customText6Enabled", false).toBool();
    if (enabled)
    {
        QString text = settings.value("serial/customText6", "").toString();
        m_serialThread->write(text.toLocal8Bit(), m_console->getLineEndingTx());
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(text.toLocal8Bit());
        }
    }
}

void MainWindow::onSendModeComboBox_currentIndexChanged()
{
    Multistring::mode_t prevMode = m_sendLine.getMode();
    Multistring::mode_t mode = static_cast<Multistring::mode_t> (ui->sendModeComboBox->currentData().toInt());
//    qDebug() << __PRETTY_FUNCTION__ << "idx:" << idx << "mode:" << mode;

    m_sendLine.setString(ui->sendLineEdit->lineEdit()->text());
    // Convert base
    m_sendLine.setMode(mode);
    m_multivalidator->setMode(mode);
    /* Save current mode's history */
    saveHistory(prevMode, getCurrentHistory());
    ui->sendLineEdit->clear();
    /* Load next mode's history */
    ui->sendLineEdit->addItems(loadHistory(mode));
    ui->sendLineEdit->lineEdit()->setText(m_sendLine.getString());
}

QStringList MainWindow::getCurrentHistory()
{
    QStringList history;

    for (int i = 0; i < ui->sendLineEdit->count(); i++)
    {
        history.append(ui->sendLineEdit->itemText(i));
    }

    return history;
}

/**
 * @brief MainWindow::loadHistory Load history of sendLineEdit values
 * @param mode ASCII, hexadecimal, etc. @see Multistring::mode_t
 * @return Loaded history.
 */
QStringList MainWindow::loadHistory(Multistring::mode_t mode)
{
    QSettings settings;
    int count = settings.value(QString("console/sendLineHistoryCount%1").arg(static_cast<int>(mode)), 0).toInt();
    QStringList history;

    for (int i = 0; i < count; i++)
    {
        QString s = settings.value(QString("console/sendLineHistory%1/%2").arg(static_cast<int>(mode)).arg(i), 0).toString();
        history.append(s);
    }

    return history;
}

/**
 * @brief MainWindow::saveHistory Save history of sendLineEdit values
 * @param mode ASCII, hexadecimal, etc. @see Multistring::mode_t
 * @param history History to be saved.
 */
void MainWindow::saveHistory(Multistring::mode_t mode, const QStringList &history)
{
    QSettings settings;
    int count = history.count();
    settings.setValue(QString("console/sendLineHistoryCount%1").arg(static_cast<int>(mode)), count);

    for (int i = 0; i < count; i++)
    {
        settings.setValue(QString("console/sendLineHistory%1/%2").arg(static_cast<int>(mode)).arg(i), history[i]);
    }
}

void MainWindow::on_actionConfigure_triggered()
{
    SettingsDialog * serialSettingsDialog = new SettingsDialog(0, &m_currentSerialSettings);
    int result = serialSettingsDialog->exec();
    qDebug() << __PRETTY_FUNCTION__ << result;

    if ((result == SettingsDialog::Accepted) && (m_serialThread->isOpen()))
    {
        // Reopen port
        m_serialThread->close();
        openSerialPort();
    }

    delete serialSettingsDialog;
}

void MainWindow::on_actionShow_timestamp_triggered(bool checked)
{
    m_console->setDisplayTimestampEnabled(checked);
}

void MainWindow::on_actionSet_timestamp_color_triggered()
{
    QPalette palette = m_console->palette();
    QColor colorOriginal = palette.color(QPalette::Dark).toRgb();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose timestamp color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/timestampcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Dark, color);
        m_console->setPalette(palette);
    }
}

void MainWindow::find(QString searchStr, bool caseSens, bool wholeWords, bool regEx)
{
    QSettings settings;

    if (!searchStr.length())
    {
        searchStr = settings.value("find/text", "").toString();
        caseSens = settings.value("find/caseSens", false).toBool();
        wholeWords = settings.value("find/wholeWords", false).toBool();
        regEx = settings.value("find/regEx", false).toBool();
    }

    QTextDocument * document = m_console->document();
    QTextCursor highlightCursor(document);
#if 0
    QTextCursor cursor(document);
    cursor.beginEditBlock();
#endif

    if (m_console->textCursor().hasSelection())
    {
        highlightCursor = m_console->textCursor();
    }

    QTextDocument::FindFlags flags;
    if (wholeWords)
    {
        flags |= QTextDocument::FindWholeWords;
    }
    if (caseSens)
    {
        flags |= QTextDocument::FindCaseSensitively;
    }

    if (regEx)
    {
        QRegExp searchRegEx(searchStr);
        searchRegEx.setPatternSyntax(QRegExp::RegExp);
        if (caseSens)
        {
            searchRegEx.setCaseSensitivity(Qt::CaseSensitive);
        }
        else
        {
            searchRegEx.setCaseSensitivity(Qt::CaseInsensitive);
        }
        highlightCursor = document->find(searchRegEx, highlightCursor, flags);
    }
    else
    {
        highlightCursor = document->find(searchStr, highlightCursor, flags);
    }

    if (!highlightCursor.isNull())
    {
//        highlightCursor.movePosition(QTextCursor::NextCharacter,
//                                     QTextCursor::KeepAnchor,
//                                     searchStr.length());
        highlightCursor.select(QTextCursor::WordUnderCursor);
        qDebug() << "selected: " << highlightCursor.selectedText();
        m_console->setTextCursor(highlightCursor);
    }
    else
    {
        QMessageBox::information(this, tr("Text not found"),
                                 "The text cannot be found.");
    }

#if 0
    while (!highlightCursor.isNull() && !highlightCursor.atEnd())
    {
        highlightCursor = document->find(searchString, highlightCursor, flags);

        if (!highlightCursor.isNull())
        {
            highlightCursor.movePosition(QTextCursor::WordRight,
                                         QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(colorFormat);
            found = true;
        }
    }

    cursor.endEditBlock();
#endif
}

void MainWindow::on_actionFind_triggered()
{
    FindDialog *dialog = new FindDialog(this);
    int result = dialog->exec();

    qDebug() << "result:" << result;
    if (result == SettingsDialog::Accepted && dialog->getText().length() > 0)
    {
        find(dialog->getText(), dialog->isCaseSens(), dialog->isWholeWords(), dialog->isRegEx());
    }

    delete dialog;
}

void MainWindow::on_actionFind_next_triggered()
{
    find();
}

void MainWindow::on_actionSelectProfile_triggered()
{
    QSettings settings;

    /* Get name of profiles */
    settings.beginGroup("profile");
    QStringList settingsProfileNames = settings.childGroups();
    QStringList profileNames;
    foreach (QString profileName, settingsProfileNames)
    {
        /* Replace 0x7F to '/', because '/' is the separator... */
        profileName.replace(REPL_CHAR, SEP_CHAR);
        profileNames.append(profileName);
    }
    settings.endGroup();

    bool ok;
    /* TODO instead of InputDialog QListWidget should be used... */
    QString profileName = QInputDialog::getItem(this, tr("Select profile"), tr("Profile:"), profileNames, 0, false, &ok);

    if (ok)
    {
        /* Change back '/' to 0x7F for reading the settings */
        profileName.replace(SEP_CHAR, REPL_CHAR);

        m_currentSerialSettings.loadSettings(profileName);
        if (m_serialThread->isOpen())
        {
            /* Post has already opened, re-open with the new settings */
            m_serialThread->close();
            openSerialPort();
        }
    }
}
