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
    console = new Console;
    enableConsole(false);
    ui->consoleWidget->addWidget(console);
    ui->hexRadioButton->setChecked(true); // FIXME load/save value
    ui->hexRadioButton->hide();
    ui->decRadioButton->hide();
    ui->binRadioButton->hide();
    console->setLineEndingRx (settings.value("serial/lineEndingRx", console->getLineEndingRx ()).toString ());
    console->setLineEndingTx (settings.value("serial/lineEndingTx", console->getLineEndingTx ()).toString ());
    console->setDataSizeLimit (settings.value("serial/dataSizeLimit", console->getDataSizeLimit ()).toInt ());
    console->setDisplaySize (settings.value("serial/displaySize", console->getDisplaySize ()).toInt ());
    console->setHexWrap (settings.value("serial/hexWrap", console->getHexWrap ()).toInt ());

    serial = new QSerialPort(this);

    serialSettings = new SettingsDialog;

    ui->actionLocal_echo->setChecked(settings.value("serial/localEchoEnabled", true).toBool());
    bool viewSendInput = settings.value("console/viewSendInput", true).toBool();
    ui->actionViewSendInput->setChecked(viewSendInput);
    on_actionViewSendInput_triggered(viewSendInput);
    ui->actionQuit->setEnabled(true);
    ui->sendLineEdit->setValidator(new HexValidator (this));

    initActionsConnections();

    MY_ASSERT(connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError))));


    MY_ASSERT(connect(serial, SIGNAL(readyRead()), this, SLOT(readData())));

    MY_ASSERT(connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray))));
}


MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("window/width", size().width());
    settings.setValue("window/height", size().height());
    settings.setValue("serial/localEchoEnabled", ui->actionLocal_echo->isChecked());
    settings.setValue("console/viewSendInput", ui->actionViewSendInput->isChecked());
    delete serialSettings;
    delete ui;
}

void MainWindow::enableConsole(bool enable)
{
    console->setReadOnly (!enable);
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
    SettingsDialog::SerialSettings p = serialSettings->serialSettings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite))
    {
        enableConsole(true);
        //console->setLocalEchoEnabled(p.localEchoEnabled);
        console->setLocalEchoEnabled(ui->actionLocal_echo->isChecked());
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
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}



void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->close();
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
    serial->write(data);
}



void MainWindow::readData()
{
    QByteArray data = serial->readAll();
//    qDebug() << __FUNCTION__ << data;
    /* Receive serial data and show on console */
    console->putData(data);
}



void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::initActionsConnections()
{
    MY_ASSERT(connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort())));
    MY_ASSERT(connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort())));
    MY_ASSERT(connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close())));
    MY_ASSERT(connect(ui->actionConfigure, SIGNAL(triggered()), serialSettings, SLOT(show())));
    MY_ASSERT(connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear())));
    MY_ASSERT(connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about())));
    MY_ASSERT(connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt())));
}

void MainWindow::on_actionLocal_echo_triggered(bool checked)
{
    console->setLocalEchoEnabled(checked);
}

void MainWindow::on_actionSet_font_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, console->document()->defaultFont());

    if (ok)
    {
        console->document()->setDefaultFont(font);
        QSettings settings;
        settings.setValue("console/font", font.toString());
    }
}

void MainWindow::on_actionSet_background_color_triggered()
{
    QPalette palette = console->palette();
    QColor colorOriginal = palette.color(QPalette::Base).toRgb();
    QColor color;
    color = QColorDialog::getColor(colorOriginal, this, "Choose background color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/bgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Base, color);
        console->setPalette(palette);
    }
}

void MainWindow::on_actionSet_foreground_color_triggered()
{
    QPalette palette = console->palette();
    QColor colorOriginal = palette.color(QPalette::Text).toRgb();
    QColor color;
//    qDebug() << "fgcolor orig" << colorOriginal;
    color = QColorDialog::getColor(colorOriginal, this, "Choose foreground color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/fgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Text, color);
        console->setPalette(palette);
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
        serial->write(data);
        if (ui->actionLocal_echo->isChecked())
        {
            console->putData(data);
        }
    }
}

void MainWindow::on_actionStop_update_triggered(bool checked)
{
    console->setUpdateEnabled(!checked);
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
    console->setDisplayHexValuesEnabled (checked);
}

void MainWindow::on_actionConfigure_console_triggered()
{
    ConsoleSettingsDialog *dialog = new ConsoleSettingsDialog(this);

    dialog->setLineEndingRx(console->getLineEndingRx ());
    dialog->setLineEndingTx(console->getLineEndingTx ());
    dialog->setDataBufferSize(console->getDataSizeLimit () >> 20);
    dialog->setDisplaySize(console->getDisplaySize ());
    dialog->setHexWrap (console->getHexWrap ());

    int result = dialog->exec();
//    qDebug() << __PRETTY_FUNCTION__ << result;

    if (result == ConsoleSettingsDialog::Accepted)
    {
        QString lineEndingRx = dialog->getLineEndingRx();
        QString lineEndingTx = dialog->getLineEndingTx();
        int dataSizeLimit = dialog->getDataBufferSize() << 20;
        int displaySize = dialog->getDisplaySize();
        int hexWrap = dialog->getHexWrap();
        console->setLineEndingRx(lineEndingRx);
        console->setLineEndingTx(lineEndingTx);
        console->setDataSizeLimit(dataSizeLimit);
        console->setDisplaySize (displaySize);
        console->setHexWrap (hexWrap);
        QSettings settings;
        settings.setValue("serial/lineEndingRx", lineEndingRx);
        settings.setValue("serial/lineEndingTx", lineEndingTx);
        settings.setValue("serial/dataSizeLimit", dataSizeLimit);
        settings.setValue("serial/displaySize", displaySize);
        settings.setValue("serial/hexWrap", hexWrap);
    }

    delete dialog;
}
