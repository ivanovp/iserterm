/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include "consolesettingsdialog.h"
#include "ui_consolesettingsdialog.h"
#include "multivalidator.h"

#include <QDebug>
#include <QLineEdit>
#include <QSettings>

ConsoleSettingsDialog::ConsoleSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleSettingsDialog)
{
    ui->setupUi(this);

    QSettings settings;
    resize(settings.value("consolesettingsdialog/width", 356).toInt(), settings.value("consolesettingsdialog/height", 260).toInt());

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

    /* Hide unused buttons */
    ui->text1Button->hide();
    ui->text2Button->hide();
    ui->text3Button->hide();
    ui->text4Button->hide();
    ui->text5Button->hide();
    ui->text6Button->hide();
}

ConsoleSettingsDialog::~ConsoleSettingsDialog()
{
    QSettings settings;
    settings.setValue("consolesettingsdialog/width", size().width());
    settings.setValue("consolesettingsdialog/height", size().height());

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

int ConsoleSettingsDialog::getHexWrap()
{
    return ui->hexWrapSpinBox->value ();
}

void ConsoleSettingsDialog::setHexWrap(const int hexWrap)
{
    ui->hexWrapSpinBox->setValue (hexWrap);
}

int ConsoleSettingsDialog::getDelayAfterSendByte()
{
    return ui->delayAfterSendByteSpinBox->value ();
}

void ConsoleSettingsDialog::setDelayAfterSendByte(const int delayAfterSendByte)
{
    ui->delayAfterSendByteSpinBox->setValue (delayAfterSendByte);
}

int ConsoleSettingsDialog::getDelayAfterSendNewLine()
{
    return ui->delayAfterSendNewLineSpinBox->value ();
}

void ConsoleSettingsDialog::setDelayAfterSendNewLine(const int delayAfterSendNewline)
{
    ui->delayAfterSendNewLineSpinBox->setValue (delayAfterSendNewline);
}

QString ConsoleSettingsDialog::getCustomText(int idx)
{
    QString customText;

    switch (idx)
    {
        case 0:
            customText = ui->text1Edit->toPlainText();
            break;
        case 1:
            customText = ui->text2Edit->toPlainText();
            break;
        case 2:
            customText = ui->text3Edit->toPlainText();
            break;
        case 3:
            customText = ui->text4Edit->toPlainText();
            break;
        case 4:
            customText = ui->text5Edit->toPlainText();
            break;
        case 5:
            customText = ui->text6Edit->toPlainText();
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid index!";
            Q_ASSERT(0);
            break;
    }

    return customText;
}

void ConsoleSettingsDialog::setCustomText(int idx, const QString &customText)
{
    switch (idx)
    {
        case 0:
            ui->text1Edit->setPlainText(customText);
            break;
        case 1:
            ui->text2Edit->setPlainText(customText);
            break;
        case 2:
            ui->text3Edit->setPlainText(customText);
            break;
        case 3:
            ui->text4Edit->setPlainText(customText);
            break;
        case 4:
            ui->text5Edit->setPlainText(customText);
            break;
        case 5:
            ui->text6Edit->setPlainText(customText);
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid index!";
            Q_ASSERT(0);
            break;
    }
}

bool ConsoleSettingsDialog::isCustomTextEnabled(int idx)
{
    bool enabled = false;

    switch (idx)
    {
        case 0:
            enabled = ui->text1CheckBox->isChecked();
            break;
        case 1:
            enabled = ui->text2CheckBox->isChecked();
            break;
        case 2:
            enabled = ui->text3CheckBox->isChecked();
            break;
        case 3:
            enabled = ui->text4CheckBox->isChecked();
            break;
        case 4:
            enabled = ui->text5CheckBox->isChecked();
            break;
        case 5:
            enabled = ui->text6CheckBox->isChecked();
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid index!";
            Q_ASSERT(0);
            break;
    }

    return enabled;
}

void ConsoleSettingsDialog::setCustomTextEnabled(int idx, bool enable)
{
    switch (idx)
    {
        case 0:
            ui->text1CheckBox->setChecked(enable);
            break;
        case 1:
            ui->text2CheckBox->setChecked(enable);
            break;
        case 2:
            ui->text3CheckBox->setChecked(enable);
            break;
        case 3:
            ui->text4CheckBox->setChecked(enable);
            break;
        case 4:
            ui->text5CheckBox->setChecked(enable);
            break;
        case 5:
            ui->text6CheckBox->setChecked(enable);
            break;
        default:
            qCritical() << __PRETTY_FUNCTION__ << "invalid index!";
            Q_ASSERT(0);
            break;
    }
}

void ConsoleSettingsDialog::on_lineEndingTxComboBox_currentIndexChanged(int index)
{
    bool isCustomLineEndingTx = (index == ui->lineEndingTxComboBox->count() - 1);
    ui->lineEndingTxComboBox->setEditable(isCustomLineEndingTx);
    if (isCustomLineEndingTx)
    {
        ui->lineEndingTxComboBox->lineEdit ()->setValidator (new MultiValidator(this));
        ui->lineEndingTxComboBox->clearEditText ();
    }
}

void ConsoleSettingsDialog::on_lineEndingRxComboBox_currentIndexChanged(int index)
{
    bool isCustomLineEndingRx = (index == ui->lineEndingRxComboBox->count() - 1);
    ui->lineEndingRxComboBox->setEditable(isCustomLineEndingRx);
    if (isCustomLineEndingRx)
    {
        ui->lineEndingRxComboBox->lineEdit ()->setValidator (new MultiValidator(this));
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
            qWarning() << "Cannot convert string to number!";
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
        s.sprintf ("%02X", static_cast<int> (binString[i].toLatin1 ()));
        hexString += s;
    }

    return hexString;
}
