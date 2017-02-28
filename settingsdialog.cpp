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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "common.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QSettings>

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    intValidator = new QIntValidator(0, 4000000, this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

//    MY_ASSERT(connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
//            this, SLOT(on_buttonBox_clicked(QAbstractButton*))));
    MY_ASSERT(connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(showPortInfo(int))));
    MY_ASSERT(connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int))));
    MY_ASSERT(connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomDevicePathPolicy(int))));
    MY_ASSERT(connect(&m_timer, SIGNAL(timeout()), this, SLOT(fillPortsInfo())));

    fillPortsParameters();
    fillPortsInfo();

    QSettings settings;
    // FIXME custom serial name
    ui->serialPortInfoListBox->setCurrentText(settings.value ("serial/name", currentSettings.name).toString());
    // FIXME custom baud rate!
    ui->baudRateBox->setCurrentText(settings.value ("serial/baudRate", currentSettings.baudRate).toString());
    ui->dataBitsBox->setCurrentText(settings.value ("serial/dataBits", currentSettings.dataBits).toString());
    ui->parityBox->setCurrentText(settings.value ("serial/parity", currentSettings.parity).toString());
    ui->stopBitsBox->setCurrentText(settings.value ("serial/stopBits", currentSettings.stopBits).toString());
    ui->flowControlBox->setCurrentText(settings.value ("serial/flowControl", currentSettings.flowControl).toString());
//    settings.value ("serial/localEchoEnabled", currentSettings.localEchoEnabled);

    updateSettings();

    m_timer.setInterval(1000);
    m_timer.start();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
    ui = NULL;
}

SettingsDialog::SerialSettings SettingsDialog::serialSettings() const
{
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    hide();

    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        setResult(Accepted);
        /* Ok button was pressed */
        updateSettings();
        QSettings settings;
        settings.setValue ("serial/name", currentSettings.name);
        settings.setValue ("serial/baudRate", currentSettings.baudRate);
        settings.setValue ("serial/dataBits", currentSettings.dataBits);
        settings.setValue ("serial/parity", currentSettings.stringParity);
        settings.setValue ("serial/stopBits", currentSettings.stopBits);
        settings.setValue ("serial/flowControl", currentSettings.stringFlowControl);
//        settings.setValue ("serial/localEchoEnabled", currentSettings.localEchoEnabled);
    }
    else
    {
        setResult(Rejected);
    }
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate)
    {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
    {
        ui->serialPortInfoListBox->clearEditText();
    }
}

void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(tr("Custom"));

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("No handshake"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

/**
 * @brief SettingsDialog::fillPortsInfo
 * Update list of serial ports. It is called in every second.
 */
void SettingsDialog::fillPortsInfo()
{
    bool update = false;
    QString description;
    QString manufacturer;
    QString serialNumber;

    /* Check if serial ports have changed */
    if (m_availablePorts.size() != QSerialPortInfo::availablePorts().size())
    {
        update = true;
    }
    else
    {
        for (int i = 0; i < QSerialPortInfo::availablePorts().size(); i++)
        {
            const QSerialPortInfo info1 = m_availablePorts[i];
            const QSerialPortInfo info2 = QSerialPortInfo::availablePorts()[i];
            if (info1.portName() != info2.portName()
                    || info1.description() != info2.description()
                    || info1.manufacturer() != info2.manufacturer()
#if QT_VERSION >= 0x050300
                    || info1.serialNumber() != info2.serialNumber()
#endif
                    || info1.systemLocation() != info2.systemLocation()
                    || info1.vendorIdentifier() != info2.vendorIdentifier()
                    || info1.productIdentifier() != info2.productIdentifier())
            {
                update = true;
                break;
            }
        }
    }

    /* Update of list is necessary */
    if (update)
    {
        m_availablePorts = QSerialPortInfo::availablePorts();
        ui->serialPortInfoListBox->clear();
        foreach (const QSerialPortInfo &info, m_availablePorts)
        {
            QStringList list;
            description = info.description();
            manufacturer = info.manufacturer();
#if QT_VERSION >= 0x050300
            serialNumber = info.serialNumber();
#else
            serialNumber.clear();
#endif
            list << info.portName()
                 << (!description.isEmpty() ? description : blankString)
                 << (!manufacturer.isEmpty() ? manufacturer : blankString)
                 << (!serialNumber.isEmpty() ? serialNumber : blankString)
                 << info.systemLocation()
                 << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
                 << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

            ui->serialPortInfoListBox->addItem(list.first(), list);
        }

        ui->serialPortInfoListBox->addItem(tr("Custom"));
    }
}

void SettingsDialog::updateSettings()
{
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (ui->baudRateBox->currentIndex() == ui->baudRateBox->count() - 1)
    {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    }
    else
    {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

//    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();
}
