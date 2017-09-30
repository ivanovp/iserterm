/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015-2016 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef CONSOLESETTINGSDIALOG_H
#define CONSOLESETTINGSDIALOG_H

#include <QDialog>
#include <QCompleter>

namespace Ui {
    class ConsoleSettingsDialog;
    }

class ConsoleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConsoleSettingsDialog(QWidget *parent = 0);
    ~ConsoleSettingsDialog();

    QString getLineEndingTx();
    void setLineEndingTx(const QString& lineEndingTx);

    QString getLineEndingRx();
    void setLineEndingRx(const QString& lineEndingRx);

    int getDataBufferSize();
    void setDataBufferSize(const int dataBufferSize);

    int getDisplaySize();
    void setDisplaySize(const int displayLines);

    int getHexWrap();
    void setHexWrap(const int hexWrap);

    int getDelayAfterSendByte();
    void setDelayAfterSendByte(const int delayAfterSendByte);

    int getDelayAfterSendNewLine();
    void setDelayAfterSendNewLine(const int delayAfterSendNewline);

    QString getTimestampFormatString();
    void setTimestampFormatString(const QString& formatString);

    QString getCustomText(int idx);
    void setCustomText(int idx, const QString& customText);

    bool isCustomTextEnabled(int idx);
    void setCustomTextEnabled(int idx, bool enable = true);

    QCompleter::CompletionMode getCompletionMode();
    void setCompletionMode(QCompleter::CompletionMode mode);

    Qt::CaseSensitivity getCompletionCaseSensitivity();
    void setCompletionCaseSensitivity(Qt::CaseSensitivity caseSensitivity);
private slots:
    void on_lineEndingTxComboBox_currentIndexChanged(int index);
    void on_lineEndingRxComboBox_currentIndexChanged(int index);
    QString hexString2Bin(const QString& hexString);
    QString bin2hexString(const QString& binString);

    void on_timestampComboBox_currentIndexChanged(int index);

private:
    Ui::ConsoleSettingsDialog *ui;
    QString m_customTimestampFormatString;
};

#endif // CONSOLESETTINGSDIALOG_H
