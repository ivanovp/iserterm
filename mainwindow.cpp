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

#include <QDebug>
#include <QMessageBox>
#include <QFontDialog>
#include <QSettings>
#include <QColorDialog>
#include <QtSerialPort/QSerialPort>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QSettings settings;
//! [0]
    ui->setupUi(this);
    setWindowIcon (QIcon (":/images/iserterm.png"));
    resize(settings.value("window/width", 500).toInt(), settings.value("window/height", 300).toInt());
    console = new Console;
    enableConsole(false);
    ui->consoleWidget->addWidget(console);
//! [1]
    serial = new QSerialPort(this);
//! [1]
    serialSettings = new SettingsDialog;

    ui->actionLocal_echo->setChecked(settings.value("serial/localEchoEnabled", true).toBool());
    ui->actionQuit->setEnabled(true);
    ui->hexInputLineEdit->setValidator(new HexValidator (this));

    initActionsConnections();

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

//! [2]
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
//! [2]
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("window/width", size().width());
    settings.setValue("window/height", size().height());
    settings.setValue("serial/localEchoEnabled", ui->actionLocal_echo->isChecked());
    delete serialSettings;
    delete ui;
}

void MainWindow::enableConsole(bool enable)
{
    console->setReadOnly (!enable);
//    console->setEnabled(enable);
    ui->hexInputLabel->setEnabled(enable);
    ui->hexInputLineEdit->setEnabled(enable);
    ui->actionConnect->setEnabled(!enable);
    ui->actionDisconnect->setEnabled(enable);
    ui->actionConfigure->setEnabled(!enable);
}

//! [4]
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
        ui->statusBar->showMessage(tr("Connected to %1: %2, %3%4%5, flow control: %6")
                                   .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                   .arg(p.stringParity[0]).arg(p.stringStopBits).arg(p.stringFlowControl));
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->close();
    }
    enableConsole(false);
    ui->statusBar->showMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About iSerTerm"),
                       tr("iSerTerm v0.1\n"
                          "Compiled on " __DATE__ " " __TIME__ ".\n"
                          "RS-232 serial terminal software based on Simple Terminal example.\n"
                          "Copyright (C) Peter Ivanov <ivanovp@gmail.com>, 2015\n"
                          "\n"
                          "Simple Terminal authors:\n"
                          "Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>\n"
                          "Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>\n"
                          ));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
//    qDebug() << __FUNCTION__ << data;
    /* Send user input */
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    qDebug() << __FUNCTION__ << data;
    /* Receive serial data and show on console */
    console->putData(data);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), serialSettings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
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
    qDebug() << "fgcolor orig" << colorOriginal;
    color = QColorDialog::getColor(colorOriginal, this, "Choose foreground color");
    if (color.isValid())
    {
        QSettings settings;
        settings.setValue("console/fgcolor", color.name(QColor::HexArgb));
        palette.setColor(QPalette::Text, color);
        console->setPalette(palette);
    }
}

void MainWindow::on_hexInputLineEdit_returnPressed()
{
    QString str = ui->hexInputLineEdit->text();
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
            qDebug() << "Cannot convert string to number!";
        }
        str.remove(0, 2);
    }
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
