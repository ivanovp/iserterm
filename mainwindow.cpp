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

#include <QDebug>
#include <QMessageBox>
#include <QFontDialog>
#include <QSettings>
#include <QColorDialog>
#include <QtSerialPort/QSerialPort>
#include <QProgressBar>
#include <QFlags>
#include <QFileDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSettings settings;

    ui->setupUi(this);

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
    ui->sendModeComboBox->addItem("ASCII", QVariant(Multistring::ASCII));
    ui->sendModeComboBox->addItem("Hex", QVariant(Multistring::Hexadecimal));
    ui->sendModeComboBox->addItem("Dec", QVariant(Multistring::Decimal));
    ui->sendModeComboBox->addItem("Bin", QVariant(Multistring::Binary));
//    ui->sendButtonGroup->setId(ui->asciiRadioButton, Multistring::ASCII);
//    ui->sendButtonGroup->setId(ui->hexRadioButton, Multistring::Hexadecimal);
//    ui->sendButtonGroup->setId(ui->decRadioButton, Multistring::Decimal);
//    ui->sendButtonGroup->setId(ui->binRadioButton, Multistring::Binary);

    switch (settings.value("console/sendModeComboBox", 0).toInt())
    {
        default:
        case 0:
            ui->sendModeComboBox->setCurrentText("ASCII");
            break;

        case 1:
            ui->sendModeComboBox->setCurrentText("Hex");
            break;

        case 2:
            ui->sendModeComboBox->setCurrentText("Dec");
            break;

        case 3:
            ui->sendModeComboBox->setCurrentText("Bin");
            break;

    }
    Multistring::mode_t mode = static_cast<Multistring::mode_t> (ui->sendModeComboBox->currentData().toInt());
    m_multivalidator->setMode(mode);
    m_sendLine.setMode(mode);

    ui->sendLineEdit->setValidator(m_multivalidator);
    ui->sendLineEdit->setEditable(true);
    ui->sendLineEdit->addItems(loadHistory(mode));
    ui->sendLineEdit->lineEdit()->setText(settings.value("console/sendLineEdit").toString());
    ui->sendLineEdit->installEventFilter(new ShiftDelEventFilter);
    ui->eolCheckBox->setChecked(settings.value("console/eolCheckBox").toBool());
    m_console->setLineEndingRx (settings.value("serial/lineEndingRx", m_console->getLineEndingRx ()).toString ());
    m_console->setLineEndingTx (settings.value("serial/lineEndingTx", m_console->getLineEndingTx ()).toString ());
    m_console->setDataSizeLimit (settings.value("serial/dataSizeLimit", m_console->getDataSizeLimit ()).toInt ());
    m_console->setDisplaySize (settings.value("serial/displaySize", m_console->getDisplaySize ()).toInt ());
    m_console->setHexWrap (settings.value("serial/hexWrap", m_console->getHexWrap ()).toInt ());

    m_serialThread = new SerialThread;
    m_serialThread->setDelayAfterBytes_ms (settings.value ("serial/delayAfterBytes_ms", m_serialThread->getDelayAfterBytes_ms ()).toInt());
    m_serialThread->setDelayAfterChr_ms(settings.value ("serial/delayAfterNewline_ms", m_serialThread->getDelayAfterChr_ms()).toInt(),
                                        m_console->getLineEndingTx().right(1).toLatin1());
    m_serialThread->start (QThread::NormalPriority);
    m_serialSettings = new SettingsDialog;

    ui->actionLocal_echo->setChecked(settings.value("serial/localEchoEnabled", true).toBool());
    bool viewLineStatus = settings.value("console/showLineStatus", true).toBool();
    ui->actionShow_line_status->setChecked(viewLineStatus);
    on_actionShow_line_status_triggered(viewLineStatus);
    bool viewSendInput = settings.value("console/viewSendInput", true).toBool();
    ui->actionViewSendInput->setChecked(viewSendInput);
    on_actionViewSendInput_triggered(viewSendInput);
    ui->actionQuit->setEnabled(true);
    m_progressBar = new QProgressBar(this);
    m_progressBar->hide();
    ui->statusBar->addPermanentWidget(m_progressBar);

    MY_ASSERT(connect(ui->sendLineEdit->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onSendLineEdit_returnPressed())));
    MY_ASSERT(connect(ui->sendModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSendModeComboBox_currentIndexChanged())));

    MY_ASSERT(connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort())));
    MY_ASSERT(connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort())));
    MY_ASSERT(connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close())));
    MY_ASSERT(connect(ui->actionConfigure, SIGNAL(triggered()), m_serialSettings, SLOT(show())));
    MY_ASSERT(connect(ui->actionClear, SIGNAL(triggered()), m_console, SLOT(clear())));
    MY_ASSERT(connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about())));
    MY_ASSERT(connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt())));

    MY_ASSERT(connect(m_serialThread, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError))));
    MY_ASSERT(connect(m_serialThread, SIGNAL(readyRead()), this, SLOT(readData())));
    MY_ASSERT(connect(m_serialThread, SIGNAL(progress(QString,int)), this, SLOT(serialProgress(QString,int))));
    MY_ASSERT(connect(m_serialThread, SIGNAL(finish()), this, SLOT(serialFinish())));
    MY_ASSERT(connect(m_serialThread, SIGNAL(pinoutSignalsChanged(QSerialPort::PinoutSignals)),
                      this, SLOT(serialPinoutsChanged(QSerialPort::PinoutSignals))));

    MY_ASSERT(connect(m_console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray))));
}


MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("window/width", size().width());
    settings.setValue("window/height", size().height());
    settings.setValue("serial/localEchoEnabled", ui->actionLocal_echo->isChecked());
    settings.setValue("console/viewSendInput", ui->actionViewSendInput->isChecked());
    settings.setValue("console/showLineStatus", ui->actionShow_line_status->isChecked());
    settings.setValue("console/sendModeComboBox", ui->sendModeComboBox->currentIndex());
    settings.setValue("console/sendLineEdit", ui->sendLineEdit->lineEdit()->text());
    settings.setValue("console/eolCheckBox", ui->eolCheckBox->isChecked());
    saveHistory(m_sendLine.getMode(), getCurrentHistory());
    if (m_serialThread)
    {
        m_serialThread->stop(250);
    }
    delete m_serialThread;
    m_serialThread = NULL;
    delete m_serialSettings;
    m_serialSettings = NULL;
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
    ui->actionConfigure->setEnabled(!enable);
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
            QString s = QString("Send custom text %1:\n%2").arg(idx).arg(text);
            action->setToolTip(s);
        }
    }
}

void MainWindow::openSerialPort()
{
    SettingsDialog::SerialSettings p = m_serialSettings->serialSettings();
    m_serialThread->setPortName(p.name);
    if (m_serialThread->open(QIODevice::ReadWrite))
    {
        // Parameters shall be set after open on Qt 5.3!
        m_serialThread->setBaudRate(p.baudRate);
        m_serialThread->setDataBits(p.dataBits);
        m_serialThread->setParity(p.parity);
        m_serialThread->setStopBits(p.stopBits);
        m_serialThread->setFlowControl(p.flowControl);
        setEnableConsole(true);
        //console->setLocalEchoEnabled(p.localEchoEnabled);
        m_console->setLocalEchoEnabled(ui->actionLocal_echo->isChecked());
        ui->statusBar->showMessage(tr("Connected to %1: %2, %3%4%5, %6")
                                   .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                   .arg(p.stringParity[0]).arg(p.stringStopBits).arg(p.stringFlowControl));
        QString title = QString("%1 - %2, %3, %4%5%6, %7").arg(VER_PRODUCTNAME_STR).arg(p.name)
            .arg(p.stringBaudRate).arg(p.stringDataBits).arg(p.stringParity[0]).arg(p.stringStopBits)
            .arg(p.stringFlowControl);
        setWindowTitle(title);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), m_serialThread->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort(const QString &errorMsg)
{
    if (m_serialThread->isOpen())
    {
        m_serialThread->close();
    }
    setEnableConsole(false);
    if (errorMsg.isEmpty())
    {
        ui->statusBar->showMessage(tr("Disconnected"));
    }
    else
    {
        ui->statusBar->showMessage(tr("Disconnected: %1").arg(errorMsg));
    }
    setWindowTitle(VER_PRODUCTNAME_STR);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About"),
                       QString(tr("%1 v%2.%3.%4\n"
                          "Compiled on " __DATE__ " " __TIME__ ".\n"
                          "RS-232 serial terminal software based on Simple Terminal example.\n"
                          "Copyright (C) Peter Ivanov <ivanovp@gmail.com>, 2015-2016\n"
                          "\n"
                          "Simple Terminal authors:\n"
                          "Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>\n"
                          "Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>\n"
                          )).arg(VER_PRODUCTNAME_STR).arg(VER_PRODUCT_MAJOR).arg(VER_PRODUCT_MINOR).arg(VER_PRODUCT_RELEASE));
}

void MainWindow::writeData(const QByteArray &data)
{
    /* Send user input */
    m_serialThread->write (data);
}



void MainWindow::readData()
{
    QByteArray data = m_serialThread->readAll();
    /* Receive serial data and show on console */
    m_console->putData(data);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        closeSerialPort(m_serialThread->errorString());
    }
    else
    {
//      qDebug() << __PRETTY_FUNCTION__ << error;
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
    QPalette palette = m_console->palette();
    QColor colorOriginal = palette.color(QPalette::Base).toRgb();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose background color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/bgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Base, color);
        m_console->setPalette(palette);
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

    m_sendLine.setString(str);
    data = m_sendLine.getByteArray();
    if (ui->eolCheckBox->isChecked())
    {
        data.append(m_console->getLineEndingTx());
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

    dialog->setLineEndingRx(m_console->getLineEndingRx ());
    dialog->setLineEndingTx(m_console->getLineEndingTx ());
    dialog->setDataBufferSize(m_console->getDataSizeLimit () >> 20);
    dialog->setDisplaySize(m_console->getDisplaySize ());
    dialog->setHexWrap(m_console->getHexWrap ());
    dialog->setDelayAfterSendByte(m_serialThread->getDelayAfterBytes_ms());
    dialog->setDelayAfterSendNewLine(m_serialThread->getDelayAfterChr_ms());
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
        QString lineEndingRx = dialog->getLineEndingRx();
        QString lineEndingTx = dialog->getLineEndingTx();
        int dataSizeLimit = dialog->getDataBufferSize() << 20;
        int displaySize = dialog->getDisplaySize();
        int hexWrap = dialog->getHexWrap();
        int delayAfterBytes_ms = dialog->getDelayAfterSendByte();
        int delayAfterNewline_ms = dialog->getDelayAfterSendNewLine();
        QSettings settings;
        for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
        {
            m_customTexts[i] = dialog->getCustomText(i);
            m_customTextsEnabled[i] = dialog->isCustomTextEnabled(i);
            setVisibleCustomText(i, m_customTextsEnabled[i], m_customTexts[i]);
            settings.setValue(QString("serial/customText%1").arg(i + 1), m_customTexts[i]);
            settings.setValue(QString("serial/customText%1Enabled").arg(i + 1), m_customTextsEnabled[i]);
        }
        m_console->setLineEndingRx(lineEndingRx);
        m_console->setLineEndingTx(lineEndingTx);
        m_console->setDataSizeLimit(dataSizeLimit);
        m_console->setDisplaySize (displaySize);
        m_console->setHexWrap (hexWrap);
        m_serialThread->setDelayAfterBytes_ms(delayAfterBytes_ms);
        m_serialThread->setDelayAfterChr_ms(delayAfterNewline_ms, lineEndingTx.right(1).toLatin1());
        settings.setValue("serial/lineEndingRx", lineEndingRx);
        settings.setValue("serial/lineEndingTx", lineEndingTx);
        settings.setValue("serial/dataSizeLimit", dataSizeLimit);
        settings.setValue("serial/displaySize", displaySize);
        settings.setValue("serial/hexWrap", hexWrap);
        settings.setValue("serial/delayAfterBytes_ms", delayAfterBytes_ms);
        settings.setValue("serial/delayAfterNewline_ms", delayAfterNewline_ms);
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
}

void MainWindow::serialFinish()
{
    m_progressBar->hide();
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
