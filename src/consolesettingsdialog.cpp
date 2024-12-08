/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#include "consolesettingsdialog.h"
#include "ui_consolesettingsdialog.h"
#include "multivalidator.h"
#include "common.h"

#include <QDebug>
#include <QLineEdit>
#include <QSettings>
#include <QCheckBox>
#include <QCompleter>
#include <QComboBox>

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

    ui->timestampComboBox->addItem("HH:mm:ss.zzz  ");
    ui->timestampComboBox->addItem("HH:mm:ss  ");
    ui->timestampComboBox->addItem("yy-MM-dd HH:mm:ss.zzz  ");
    ui->timestampComboBox->addItem("yy-MM-dd HH:mm:ss  ");
    ui->timestampComboBox->addItem("HH:mm:ss.zzz | ");
    ui->timestampComboBox->addItem("HH:mm:ss | ");
    ui->timestampComboBox->addItem("yy-MM-dd HH:mm:ss.zzz | ");
    ui->timestampComboBox->addItem("yy-MM-dd HH:mm:ss | ");
    ui->timestampComboBox->addItem (tr("Custom"));
    ui->timestampComboBox->setEditable(false);

    ui->autoLogTimestampComboBox->addItem("HH:mm:ss.zzz  ");
    ui->autoLogTimestampComboBox->addItem("HH:mm:ss  ");
    ui->autoLogTimestampComboBox->addItem("yy-MM-dd HH:mm:ss.zzz  ");
    ui->autoLogTimestampComboBox->addItem("yy-MM-dd HH:mm:ss  ");
    ui->autoLogTimestampComboBox->addItem("HH:mm:ss.zzz | ");
    ui->autoLogTimestampComboBox->addItem("HH:mm:ss | ");
    ui->autoLogTimestampComboBox->addItem("yy-MM-dd HH:mm:ss.zzz | ");
    ui->autoLogTimestampComboBox->addItem("yy-MM-dd HH:mm:ss | ");
    ui->autoLogTimestampComboBox->addItem (tr("Custom"));
    ui->autoLogTimestampComboBox->setEditable(false);

    bool autoLogEnabled = settings.value("serial/autoLog", false).toBool();
    ui->autoLogCheckBox->setChecked(autoLogEnabled);
    ui->autoLogFileNameLineEdit->setText(settings.value("serial/autoLogFileName", "%Y-%m-%d_%H:%M:%S.log").toString());
    ui->autoLogFileNameLineEdit->setEnabled(autoLogEnabled);
    ui->autoLogFilePathLineEdit->setText(settings.value("serial/autoLogFilePath", "").toString());
    ui->autoLogFilePathLineEdit->setEnabled(autoLogEnabled);
    ui->autoLogFilePathBrowseButton->setEnabled(autoLogEnabled);
    ui->autoLogTimestampComboBox->setEnabled(autoLogEnabled);

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
    ui = NULL;
}

int ConsoleSettingsDialog::getAutoWrapColumn()
{
    return ui->autoWrapSpinBox->value();
}

void ConsoleSettingsDialog::setAutoWrapColumn(int autoWrapColumn)
{
    ui->autoWrapSpinBox->setValue(autoWrapColumn);
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
        lineEndingTx = ui->lineEndingTxComboBox->currentData (Qt::UserRole).toString ();
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
        ui->lineEndingTxComboBox->setEditable(true);
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
        lineEndingRx = ui->lineEndingRxComboBox->currentData (Qt::UserRole).toString ();
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
        ui->lineEndingRxComboBox->setEditable(true);
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

QString ConsoleSettingsDialog::getTimestampFormatString()
{
    QString timestamp;
    if (ui->timestampComboBox->currentIndex () == ui->timestampComboBox->count () - 1)
    {
        timestamp = ui->timestampComboBox->currentData (Qt::DisplayRole).toString ();
        if (timestamp == tr("Custom"))
        {
            /* Timestamp was not edited */
            timestamp = m_customTimestampFormatString;
        }
    }
    else
    {
        timestamp = ui->timestampComboBox->currentText ();
    }
    return timestamp;
}

void ConsoleSettingsDialog::setTimestampFormatString(const QString &formatString)
{
    bool found = false;

    for (int i = 0; i < ui->timestampComboBox->count () - 1; i++)
    {
        if (formatString == ui->timestampComboBox->itemText(i))
        {
            ui->timestampComboBox->setCurrentIndex (i);
            found = true;
        }
    }
    if (!found)
    {
        /* This is a custom timestamp */
        ui->timestampComboBox->setCurrentIndex (ui->timestampComboBox->count () - 1);
        ui->timestampComboBox->setEditText (formatString);
        ui->timestampComboBox->setEditable(true);
    }
    m_customLogTimestampFormatString = formatString;
}

QString ConsoleSettingsDialog::getLogTimestampFormatString()
{
    QString timestamp;
    if (ui->timestampComboBox->currentIndex () == ui->timestampComboBox->count () - 1)
    {
        timestamp = ui->timestampComboBox->currentData (Qt::DisplayRole).toString ();
        if (timestamp == tr("Custom"))
        {
            /* LogTimestamp was not edited */
            timestamp = m_customLogTimestampFormatString;
        }
    }
    else
    {
        timestamp = ui->timestampComboBox->currentText ();
    }
    return timestamp;
}

void ConsoleSettingsDialog::setLogTimestampFormatString(const QString &formatString)
{
    bool found = false;

    for (int i = 0; i < ui->timestampComboBox->count () - 1; i++)
    {
        if (formatString == ui->timestampComboBox->itemText(i))
        {
            ui->timestampComboBox->setCurrentIndex (i);
            found = true;
        }
    }
    if (!found)
    {
        /* This is a custom timestamp */
        ui->timestampComboBox->setCurrentIndex (ui->timestampComboBox->count () - 1);
        ui->timestampComboBox->setEditText (formatString);
        ui->timestampComboBox->setEditable(true);
    }
    m_customLogTimestampFormatString = formatString;
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

QCompleter::CompletionMode ConsoleSettingsDialog::getCompletionMode()
{
    return static_cast<QCompleter::CompletionMode>( ui->completionModeComboBox->currentIndex() );
}

void ConsoleSettingsDialog::setCompletionMode(QCompleter::CompletionMode mode)
{
    ui->completionModeComboBox->setCurrentIndex(static_cast<int>(mode));
}

Qt::CaseSensitivity ConsoleSettingsDialog::getCompletionCaseSensitivity()
{
    Qt::CaseSensitivity sens;

    if (ui->completionCaseSensCheckBox->isChecked())
    {
        sens = Qt::CaseSensitive;
    }
    else
    {
        sens = Qt::CaseInsensitive;
    }

    return sens;
}

void ConsoleSettingsDialog::setCompletionCaseSensitivity(Qt::CaseSensitivity caseSensitivity)
{
    ui->completionCaseSensCheckBox->setChecked(caseSensitivity == Qt::CaseSensitive);
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
        s.asprintf ("%02X", static_cast<int> (binString[i].toLatin1 ()));
        hexString += s;
    }

    return hexString;
}

void ConsoleSettingsDialog::on_timestampComboBox_currentIndexChanged(int index)
{
    bool isCustomTimestamp = (index == ui->timestampComboBox->count() - 1);
    ui->timestampComboBox->setEditable(isCustomTimestamp);
    if (isCustomTimestamp)
    {
        //ui->timestampComboBox->lineEdit ()->setValidator (new MultiValidator(this));
        ui->timestampComboBox->clearEditText ();
    }
}

void ConsoleSettingsDialog::on_autoLogCheckBox_stateChanged(int arg1)
{
    ui->autoLogFileNameLineEdit->setEnabled(arg1);
    ui->autoLogFilePathLineEdit->setEnabled(arg1);
    ui->autoLogFilePathBrowseButton->setEnabled(arg1);
    ui->autoLogTimestampComboBox->setEnabled(arg1);
}

void ConsoleSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        int autoWrapColumn = getAutoWrapColumn();
        QString lineEndingRx = getLineEndingRx();
        QString lineEndingTx = getLineEndingTx();
        int dataSizeLimit = getDataBufferSize() << 20;
        int displaySize = getDisplaySize();
        int hexWrap = getHexWrap();
        int delayAfterBytes_ms = getDelayAfterSendByte();
        int delayAfterNewline_ms = getDelayAfterSendNewLine();
        QString timestampFormatString = getTimestampFormatString();
        QSettings settings;
        for (int i = 0; i < CUSTOM_TEXT_NUM; i++)
        {
            settings.setValue(QString("serial/customText%1").arg(i + 1), getCustomText(i));
            settings.setValue(QString("serial/customText%1Enabled").arg(i + 1), isCustomTextEnabled(i));
        }
        settings.setValue("serial/autoWrapColumn", autoWrapColumn);
        settings.setValue("serial/lineEndingRx", lineEndingRx);
        settings.setValue("serial/lineEndingTx", lineEndingTx);
        settings.setValue("serial/dataSizeLimit", dataSizeLimit);
        settings.setValue("serial/displaySize", displaySize);
        settings.setValue("serial/hexWrap", hexWrap);
        settings.setValue("serial/delayAfterBytes_ms", delayAfterBytes_ms);
        settings.setValue("serial/delayAfterNewline_ms", delayAfterNewline_ms);
        settings.setValue("console/timestampFormatString", timestampFormatString);
        settings.setValue("completion/mode", getCompletionMode());
        settings.setValue("completion/caseSensitivity", getCompletionCaseSensitivity());
        settings.setValue("serial/autoLog", ui->autoLogCheckBox->isChecked());
        settings.setValue("serial/autoLogFileName", ui->autoLogFileNameLineEdit->text());
        settings.setValue("serial/autoLogFilePath", ui->autoLogFilePathLineEdit->text());
    }
}

