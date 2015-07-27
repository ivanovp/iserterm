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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "hexvalidator.h"
#include "common.h"
#include "version.h"
#include "consolesettingsdialog.h"
#include "serialthread.h"

#include <QDebug>
#include <QMessageBox>
#include <QFontDialog>
#include <QSettings>
#include <QColorDialog>
#include <QtSerialPort/QSerialPort>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QSettings settings;

    ui->setupUi(this);
    setWindowIcon (QIcon (":/images/iserterm.png"));
    setWindowTitle(VER_PRODUCTNAME_STR);
    resize(settings.value("window/width", 500).toInt(), settings.value("window/height", 300).toInt());
    m_console = new Console;
    enableConsole(false);
    ui->consoleWidget->addWidget(m_console);
    ui->hexRadioButton->setChecked(true); // FIXME load/save value
    ui->hexRadioButton->hide();
    ui->decRadioButton->hide();
    ui->binRadioButton->hide();
    m_console->setLineEndingRx (settings.value("serial/lineEndingRx", m_console->getLineEndingRx ()).toString ());
    m_console->setLineEndingTx (settings.value("serial/lineEndingTx", m_console->getLineEndingTx ()).toString ());
    m_console->setDataSizeLimit (settings.value("serial/dataSizeLimit", m_console->getDataSizeLimit ()).toInt ());
    m_console->setDisplaySize (settings.value("serial/displaySize", m_console->getDisplaySize ()).toInt ());
    m_console->setHexWrap (settings.value("serial/hexWrap", m_console->getHexWrap ()).toInt ());

    m_serial = new QSerialPort(this);
    m_serialThread = new SerialThread;
    m_serialThread->setSerialDevice (m_serial);
    m_serialThread->setDelayAfterBytes_us (settings.value ("serial/delayAfterBytes_us", m_serialThread->getDelayAfterBytes_us ()).toInt());
    m_serialThread->setDelayAfterChr_us(settings.value ("serial/delayAfterNewline_us", m_serialThread->getDelayAfterChr_us()).toInt(),
                                        m_console->getLineEndingTx().toLatin1());
    m_serialThread->start ();
    m_serialSettings = new SettingsDialog;

    ui->actionLocal_echo->setChecked(settings.value("serial/localEchoEnabled", true).toBool());
    bool viewSendInput = settings.value("console/viewSendInput", true).toBool();
    ui->actionViewSendInput->setChecked(viewSendInput);
    on_actionViewSendInput_triggered(viewSendInput);
    ui->actionQuit->setEnabled(true);
    ui->sendLineEdit->setValidator(new HexValidator (this));

    initActionsConnections();

    MY_ASSERT(connect(m_serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError))));


    MY_ASSERT(connect(m_serial, SIGNAL(readyRead()), this, SLOT(readData())));

    MY_ASSERT(connect(m_console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray))));
}


MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("window/width", size().width());
    settings.setValue("window/height", size().height());
    settings.setValue("serial/localEchoEnabled", ui->actionLocal_echo->isChecked());
    settings.setValue("console/viewSendInput", ui->actionViewSendInput->isChecked());
    if (m_serialThread)
    {
        m_serialThread->stop(100);
    }
    delete m_serialThread;
    delete m_serialSettings;
    delete ui;
}

void MainWindow::enableConsole(bool enable)
{
    m_console->setReadOnly (!enable);
//    console->setEnabled(enable);
//    ui->sendLayout->setEnabled(enable);
    ui->sendLabel->setEnabled(enable);
    ui->sendLineEdit->setEnabled(enable);
    ui->sendButton->setEnabled(enable);
    ui->actionConnect->setEnabled(!enable);
    ui->actionDisconnect->setEnabled(enable);
    ui->actionConfigure->setEnabled(!enable);
}


void MainWindow::openSerialPort()
{
    SettingsDialog::SerialSettings p = m_serialSettings->serialSettings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite))
    {
        enableConsole(true);
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
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}



void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    enableConsole(false);
    ui->statusBar->showMessage(tr("Disconnected"));
    setWindowTitle(VER_PRODUCTNAME_STR);
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About"),
                       QString(tr("%1 v%2.%3.%4\n"
                          "Compiled on " __DATE__ " " __TIME__ ".\n"
                          "RS-232 serial terminal software based on Simple Terminal example.\n"
                          "Copyright (C) Peter Ivanov <ivanovp@gmail.com>, 2015\n"
                          "\n"
                          "Simple Terminal authors:\n"
                          "Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>\n"
                          "Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>\n"
                          )).arg(VER_PRODUCTNAME_STR).arg(VER_PRODUCT_MAJOR).arg(VER_PRODUCT_MINOR).arg(VER_PRODUCT_RELEASE));
}


void MainWindow::writeData(const QByteArray &data)
{
//    qDebug() << __FUNCTION__ << data;
    /* Send user input */
    m_serialThread->addData (data);
//    m_serial->write(data);
}



void MainWindow::readData()
{
    QByteArray data = m_serial->readAll();
//    qDebug() << __FUNCTION__ << data;
    /* Receive serial data and show on console */
    m_console->putData(data);
}



void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::initActionsConnections()
{
    MY_ASSERT(connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort())));
    MY_ASSERT(connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort())));
    MY_ASSERT(connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close())));
    MY_ASSERT(connect(ui->actionConfigure, SIGNAL(triggered()), m_serialSettings, SLOT(show())));
    MY_ASSERT(connect(ui->actionClear, SIGNAL(triggered()), m_console, SLOT(clear())));
    MY_ASSERT(connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about())));
    MY_ASSERT(connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt())));
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

void MainWindow::on_sendLineEdit_returnPressed()
{
    QString str = ui->sendLineEdit->text();
    str.remove(' ');
    QByteArray data;
    while (str.length() > 1)
    {
        char c;
        bool ok;
        QString hexStr = str.left(2);
        c = hexStr.toInt(&ok, 16);
        if (ok)
        {
            data.append(c);
        }
        else
        {
            qWarning() << "Cannot convert string to number!";
        }
        str.remove(0, 2);
    }
//    qDebug() << __PRETTY_FUNCTION__ << data;
    if (data.length())
    {
        m_serial->write(data);
        if (ui->actionLocal_echo->isChecked())
        {
            m_console->putData(data);
        }
    }
}

void MainWindow::on_actionStop_update_triggered(bool checked)
{
    m_console->setUpdateEnabled(!checked);
}

void MainWindow::on_sendButton_clicked()
{
    on_sendLineEdit_returnPressed();
}

void MainWindow::on_actionViewSendInput_triggered(bool checked)
{
    ui->sendLabel->setVisible(checked);
    ui->sendLineEdit->setVisible(checked);
    ui->sendButton->setVisible(checked);
}

void MainWindow::on_actionHexadecimal_view_triggered(bool checked)
{
    m_console->setDisplayHexValuesEnabled (checked);
}

void MainWindow::on_actionConfigure_console_triggered()
{
    ConsoleSettingsDialog *dialog = new ConsoleSettingsDialog(this);

    dialog->setLineEndingRx(m_console->getLineEndingRx ());
    dialog->setLineEndingTx(m_console->getLineEndingTx ());
    dialog->setDataBufferSize(m_console->getDataSizeLimit () >> 20);
    dialog->setDisplaySize(m_console->getDisplaySize ());
    dialog->setHexWrap(m_console->getHexWrap ());
    dialog->setDelayAfterSendByte(m_serialThread->getDelayAfterBytes_us());
    dialog->setDelayAfterSendNewLine(m_serialThread->getDelayAfterChr_us());

    int result = dialog->exec();
//    qDebug() << __PRETTY_FUNCTION__ << result;

    if (result == ConsoleSettingsDialog::Accepted)
    {
        QString lineEndingRx = dialog->getLineEndingRx();
        QString lineEndingTx = dialog->getLineEndingTx();
        int dataSizeLimit = dialog->getDataBufferSize() << 20;
        int displaySize = dialog->getDisplaySize();
        int hexWrap = dialog->getHexWrap();
        int delayAfterBytes_us = dialog->getDelayAfterSendByte();
        int delayAfterNewline_us = dialog->getDelayAfterSendNewLine();
        m_console->setLineEndingRx(lineEndingRx);
        m_console->setLineEndingTx(lineEndingTx);
        m_console->setDataSizeLimit(dataSizeLimit);
        m_console->setDisplaySize (displaySize);
        m_console->setHexWrap (hexWrap);
        m_serialThread->setDelayAfterBytes_us(delayAfterBytes_us);
        m_serialThread->setDelayAfterChr_us(delayAfterBytes_us, lineEndingTx.right(1).toLatin1());
        QSettings settings;
        settings.setValue("serial/lineEndingRx", lineEndingRx);
        settings.setValue("serial/lineEndingTx", lineEndingTx);
        settings.setValue("serial/dataSizeLimit", dataSizeLimit);
        settings.setValue("serial/displaySize", displaySize);
        settings.setValue("serial/hexWrap", hexWrap);
        settings.setValue("serial/delayAfterBytes_us", delayAfterBytes_us);
        settings.setValue("serial/delayAfterNewline_us", delayAfterNewline_us);
    }

    delete dialog;
}
