/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include "consolesettingsdialog.h"
#include "ui_consolesettingsdialog.h"
#include "hexvalidator.h"

#include <QLineEdit>

ConsoleSettingsDialog::ConsoleSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleSettingsDialog)
{
    ui->setupUi(this);

    ui->lineEndingTxComboBox->addItem ("CR LF", "\r\n");
    ui->lineEndingTxComboBox->addItem ("LF CR", "\n\r");
    ui->lineEndingTxComboBox->addItem ("CR", "\r");
    ui->lineEndingTxComboBox->addItem ("LF", "\n");
    ui->lineEndingTxComboBox->addItem (tr("Custom"));
    ui->lineEndingTxComboBox->setEditable (false);
    ui->lineEndingTxComboBox->setCurrentIndex (2);

    ui->lineEndingRxComboBox->addItem ("CR LF", "\r\n");
    ui->lineEndingRxComboBox->addItem ("LF CR", "\n\r");
    ui->lineEndingRxComboBox->addItem ("CR", "\r");
    ui->lineEndingRxComboBox->addItem ("LF", "\n");
    ui->lineEndingRxComboBox->addItem (tr("Custom"));
    ui->lineEndingRxComboBox->setEditable (false);
    ui->lineEndingRxComboBox->setCurrentIndex (0);
}

ConsoleSettingsDialog::~ConsoleSettingsDialog()
{
    delete ui;
}

QString ConsoleSettingsDialog::getLineEndingTx()
{
    QString lineEndingTx;
    if (ui->lineEndingTxComboBox->currentIndex () == ui->lineEndingTxComboBox->count () - 1)
    {
        lineEndingTx = hexString2Bin (ui->lineEndingTxComboBox->currentText ());
    }
    else
    {
        lineEndingTx = ui->lineEndingTxComboBox->currentData ().toString ();
    }
    return lineEndingTx;
}

void ConsoleSettingsDialog::setLineEndingTx(const QString &lineEndingTx)
{
    bool found = false;

    for (int i = 0; i < ui->lineEndingTxComboBox->count () - 1; i++)
    {
        if (lineEndingTx == ui->lineEndingTxComboBox->itemData (i))
        {
            ui->lineEndingTxComboBox->setCurrentIndex (i);
            found = true;
        }
    }
    if (!found)
    {
        ui->lineEndingTxComboBox->setCurrentIndex (ui->lineEndingTxComboBox->count () - 1);
        ui->lineEndingTxComboBox->setEditText (bin2hexString (lineEndingTx));
    }
}

QString ConsoleSettingsDialog::getLineEndingRx()
{
    QString lineEndingRx;
    if (ui->lineEndingRxComboBox->currentIndex () == ui->lineEndingRxComboBox->count () - 1)
    {
        lineEndingRx = hexString2Bin (ui->lineEndingRxComboBox->currentText ());
    }
    else
    {
        lineEndingRx = ui->lineEndingRxComboBox->currentData ().toString ();
    }
    return lineEndingRx;
}

void ConsoleSettingsDialog::setLineEndingRx(const QString &lineEndingRx)
{
    bool found = false;

    for (int i = 0; i < ui->lineEndingRxComboBox->count () - 1; i++)
    {
        if (lineEndingRx == ui->lineEndingRxComboBox->itemData (i))
        {
            ui->lineEndingRxComboBox->setCurrentIndex (i);
            found = true;
        }
    }
    if (!found)
    {
        ui->lineEndingRxComboBox->setCurrentIndex (ui->lineEndingRxComboBox->count () - 1);
        ui->lineEndingRxComboBox->setEditText (bin2hexString (lineEndingRx));
    }
}

int ConsoleSettingsDialog::getDataBufferSize()
{
    return ui->dataBufferSizeSpinBox->value ();
}

void ConsoleSettingsDialog::setDataBufferSize(const int dataBufferSize)
{
    ui->dataBufferSizeSpinBox->setValue (dataBufferSize);
}

int ConsoleSettingsDialog::getDisplaySize()
{
    return ui->displaySizeSpinBox->value ();
}

void ConsoleSettingsDialog::setDisplaySize(const int displaySize)
{
    ui->displaySizeSpinBox->setValue (displaySize);
}

void ConsoleSettingsDialog::on_lineEndingTxComboBox_currentIndexChanged(int index)
{
    bool isCustomLineEndingTx = (index == ui->lineEndingTxComboBox->count() - 1);
    ui->lineEndingTxComboBox->setEditable(isCustomLineEndingTx);
    if (isCustomLineEndingTx)
    {
        ui->lineEndingTxComboBox->lineEdit ()->setValidator (new HexValidator(this));
        ui->lineEndingTxComboBox->clearEditText ();
    }
}

void ConsoleSettingsDialog::on_lineEndingRxComboBox_currentIndexChanged(int index)
{
    bool isCustomLineEndingRx = (index == ui->lineEndingRxComboBox->count() - 1);
    ui->lineEndingRxComboBox->setEditable(isCustomLineEndingRx);
    if (isCustomLineEndingRx)
    {
        ui->lineEndingRxComboBox->lineEdit ()->setValidator (new HexValidator(this));
        ui->lineEndingRxComboBox->clearEditText ();
    }
}

QString ConsoleSettingsDialog::hexString2Bin(const QString &hexString)
{
    QString str = hexString;
    str.remove(' ');
    QString bin;
    while (str.length() > 1)
    {
        char c;
        bool ok;
        QString hexStr = str.left(2);
        c = hexStr.toInt(&ok, 16);
        if (ok)
        {
            bin.append(c);
        }
        else
        {
            qDebug() << "Cannot convert string to number!";
        }
        str.remove(0, 2);
    }
    return bin;
}

QString ConsoleSettingsDialog::bin2hexString(const QString &binString)
{
    QString hexString, s;
    for (int i = 0; i < binString.length (); i++)
    {
        if (i)
        {
            hexString += " ";
        }
        s.sprintf ("%02X", binString[i]);
        hexString += s;
    }

    return hexString;
}
