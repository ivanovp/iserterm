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
#include <QAbstractButton>

namespace Ui {
    class ConsoleSettingsDialog;
    }

class ConsoleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConsoleSettingsDialog(QWidget *parent = 0);
    ~ConsoleSettingsDialog();

    int getAutoWrapColumn();
    void setAutoWrapColumn(int autoWrapColumn);

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

    bool isAutoLogEnabled();
    void setAutoLogEnabled(bool enabled=true);

    QString getTimestampFormatString();
    void setTimestampFormatString(const QString& formatString);

    QString getAutoLogFileName();
    void setAutoLogFileName(const QString& fileName);

    QString getAutoLogFilePath();
    void setAutoLogFilePath(const QString& filePath);

    QString getAutoLogTimestampFormatString();
    void setAutoLogTimestampFormatString(const QString& formatString);

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
    void on_autoLogCheckBox_stateChanged(int arg1);
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_autoLogTimestampComboBox_currentIndexChanged(int index);

    void on_autoLogFilePathBrowseButton_clicked();

private:
    Ui::ConsoleSettingsDialog *ui;
    QString m_customTimestampFormatString;
    QString m_customLogTimestampFormatString;
};

#endif // CONSOLESETTINGSDIALOG_H
