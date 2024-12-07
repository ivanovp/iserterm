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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "common.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent, SerialSettings *currentSerialSettings) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    m_currentSettings(currentSerialSettings)
{
    ui->setupUi(this);

    m_intValidator = new QIntValidator(0, 4000000, this);
    QLineEdit *edit = ui->baudRateBox->lineEdit();
    edit->setValidator(m_intValidator);

//    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

//    MY_ASSERT(connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
//            this, SLOT(on_buttonBox_clicked(QAbstractButton*))));
//    MY_ASSERT(connect(ui->profileListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
//                      this, SLOT(on_profileListWidget_currentItemChanged(QListWidgetItem*,QListWidgetItem*))));
//    MY_ASSERT(connect(ui->addButton, SIGNAL(clicked()), this, SLOT(on_addButton_clicked())));
//    MY_ASSERT(connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(on_removeButton_clicked())));
    MY_ASSERT(connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(showPortInfo(int))));
#if USE_POLICY_CHECK
    MY_ASSERT(connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(checkCustomBaudRatePolicy(int))));
    MY_ASSERT(connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(checkCustomDevicePathPolicy(int))));
#endif
    MY_ASSERT(connect(&m_timer, SIGNAL(timeout()), this, SLOT(fillPortsInfo())));

    ui->profileListWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    fillPortsParameters();
    fillPortsInfo();

    settings2ui(&m_currentSettings->m_serialSettings);

    QListWidgetItem *item = new QListWidgetItem(tr("Current"));
    /* Name of default item is not editable! */
//    item->setFlags(item->flags() | Qt::ItemIsEditable);
    settings2item(item, &m_currentSettings->m_serialSettings);
    ui->profileListWidget->addItem(item);
    /* First item is selected */
    ui->profileListWidget->setCurrentItem(item);

    /* Load profile items */
    QSettings settings;
    settings.beginGroup("profile");
    QStringList profileNames = settings.childGroups();
    foreach (QString profileName, profileNames)
    {
        /* Load settings with the original profile name (with 0x7F in it) */
        SerialSettings serialSettings;
        serialSettings.loadSettings(profileName);
        /* Then replace 0x7F to '/', because '/' is the separator... */
        profileName.replace(REPL_CHAR, SEP_CHAR);
        qDebug() << "profile" << profileName;

        QListWidgetItem *item = new QListWidgetItem(profileName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        settings2item(item, &serialSettings.m_serialSettings);
        ui->profileListWidget->addItem(item);
    }

    m_timer.setInterval(1000);
    m_timer.start();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
    ui = NULL;
}

SerialSettings::serialSettings_t SettingsDialog::serialSettings() const
{
    return m_currentSettings->m_serialSettings;
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
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        bool error = false;
        /* Check if profile names are not identical */
        /* Get all items in the profile list */
        QList<QListWidgetItem*> items = ui->profileListWidget->findItems("*", Qt::MatchWildcard);
        for (int i = 0; i < items.length() && !error; i++)
        {
            for (int j = i; j < items.length() && !error; j++)
            {
                if (j != i && items[i]->text() == items[j]->text())
                {
                    QMessageBox::critical(this, tr("Identical profile name found"), tr("Profile names shall be different!"));
                    error = true;
                }
            }
        }

        if (!error)
        {
            hide();

            setResult(Accepted);
            /* Ok button was pressed */
            ui2settings(&m_currentSettings->m_serialSettings);
            settings2item(ui->profileListWidget->currentItem(), &m_currentSettings->m_serialSettings);
            // saveSettings(&m_currentSettings);

            /* Remove all profiles, otherwise deleted names will not disappear! */
            QSettings settings;
            settings.beginGroup("profile");
            QStringList profileNames = settings.childGroups();
            foreach (QString profileName, profileNames)
            {
                settings.remove(profileName);
            }

            /* Save profile items */
            /* Remove current item, it has already saved as m_currentSettings */
            items.removeFirst();
            foreach (QListWidgetItem *item, items)
            {
                SerialSettings::serialSettings_t serialSettings;
                item2settings(item, &serialSettings);
                // saveSettings(&serialSettings, item->text());
            }
        }
    }
    else
    {
        hide();

        setResult(Rejected);
    }
}

#if USE_POLICY_CHECK
void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate)
    {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
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
#endif

void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    ui->baudRateBox->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    ui->baudRateBox->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    /* Non-standard */
    ui->baudRateBox->addItem(QStringLiteral("200000"), 200000);
    ui->baudRateBox->addItem(QStringLiteral("250400"), 250400);
#if USE_POLICY_CHECK
    ui->baudRateBox->addItem(tr("Custom"));
#endif

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
    ui->stopBitsBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("No handshake"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("Hardware (RTS/CTS)"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("Software (XON/XOFF)"), QSerialPort::SoftwareControl);
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

#if USE_POLICY_CHECK
        ui->serialPortInfoListBox->addItem(tr("Custom"));
#endif
    }
}

void SettingsDialog::on_profileListWidget_currentItemChanged(QListWidgetItem *item, QListWidgetItem *prevItem)
{
    if (prevItem == NULL)
    {
        /* If no item was selected, use default item */
        prevItem = ui->profileListWidget->item(0);
    }
    qDebug() << "old profile:" << prevItem->text() << "new profile:" << item->text();
    /* Copy settings from UI to m_currentSettings */
    ui2settings(&m_currentSettings->m_serialSettings);
    /* Store m_currentSettings to the previous item */
    settings2item(prevItem, &m_currentSettings->m_serialSettings);
    /* Get settings from actual item and copy to m_currentSettings */
    item2settings(item, &m_currentSettings->m_serialSettings);
    /* Display current settings on UI */
    settings2ui(&m_currentSettings->m_serialSettings);
}

void SettingsDialog::on_addButton_clicked()
{
    QString name = tr("Copy of ") + ui->profileListWidget->currentItem()->text();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    settings2item(item, &m_currentSettings->m_serialSettings);
    ui->profileListWidget->addItem(item);
}

void SettingsDialog::on_removeButton_clicked()
{
    int currentRow = ui->profileListWidget->currentRow();

    /* At least one item should remain in the list */
    /* and current setting cannot be removed */
    if (ui->profileListWidget->count() > 1
            && currentRow != 0)
    {
        QListWidgetItem *item = ui->profileListWidget->takeItem(currentRow);
        delete item;
    }
}

void SettingsDialog::ui2settings(SerialSettings::serialSettings_t *settings)
{
    settings->name = ui->serialPortInfoListBox->currentText();

#if USE_POLICY_CHECK
    if (ui->baudRateBox->currentIndex() == ui->baudRateBox->count() - 1)
    {
        settings->baudRate = ui->baudRateBox->currentText().toInt();
    }
    else
    {
        settings->baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
#else
    settings->baudRate = ui->baudRateBox->currentText().toInt();
#endif
    settings->stringBaudRate = QString::number(settings->baudRate);

    settings->dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());

    settings->parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    settings->stringParity = ui->parityBox->currentText();

    settings->stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    settings->stringStopBits = ui->stopBitsBox->currentText();

    settings->flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    settings->stringFlowControl = ui->flowControlBox->currentText();
}

void SettingsDialog::settings2ui(SerialSettings::serialSettings_t *settings)
{
#if USE_POLICY_CHECK
    ui->serialPortInfoListBox->setCurrentText(settings->name);
#else
    ui->serialPortInfoListBox->setEditText(settings->name);
#endif
#if USE_POLICY_CHECK
    ui->baudRateBox->setCurrentText(QString::number(settings->baudRate));
#else
    ui->baudRateBox->setEditText(QString::number(settings->baudRate));
#endif
    ui->dataBitsBox->setCurrentText(QString::number(settings->dataBits));
    ui->parityBox->setCurrentText(settings->stringParity);
    ui->stopBitsBox->setCurrentText(settings->stringStopBits);
    ui->flowControlBox->setCurrentText(settings->stringFlowControl);
}

/**
 * @brief SettingsDialog::settings2item Copy serial port settings from
 * serialSettings_t to QListWidgetItem.
 * @param item      Destination item
 * @param settings  Source of settings.
 */
void SettingsDialog::settings2item(QListWidgetItem *item, SerialSettings::serialSettings_t *settings)
{
    item->setData(role_name, settings->name);
    item->setData(role_baudRate, settings->baudRate);
    item->setData(role_dataBits, settings->dataBits);
    item->setData(role_parity, settings->parity);
    item->setData(role_stringParity, settings->stringParity);
    item->setData(role_stopBits, settings->stopBits);
    item->setData(role_stringStopBits, settings->stringStopBits);
    item->setData(role_flowControl, settings->flowControl);
    item->setData(role_stringFlowControl, settings->stringFlowControl);
}

/**
 * @brief SettingsDialog::item2settings Copy serial port settings from
 * QListWidgetItem to serialSettings_t.
 * @param item      Source item.
 * @param settings  Destination of settings.
 */
void SettingsDialog::item2settings(QListWidgetItem *item, SerialSettings::serialSettings_t *settings)
{
    settings->name = item->data(role_name).toString();
    settings->baudRate = item->data(role_baudRate).toInt();
    settings->dataBits = static_cast<QSerialPort::DataBits> (item->data(role_dataBits).toInt());
    settings->parity = static_cast<QSerialPort::Parity> (item->data(role_parity).toInt());
    settings->stringParity = item->data(role_stringParity).toString();
    settings->stopBits = static_cast<QSerialPort::StopBits> (item->data(role_stopBits).toInt());
    settings->stringStopBits = item->data(role_stringStopBits).toString();
    settings->flowControl = static_cast<QSerialPort::FlowControl> (item->data(role_flowControl).toInt());
    settings->stringFlowControl = item->data(role_stringFlowControl).toString();
}

