/****************************************************************************
**
** iSerTerm - RS-232 Serial terminal
** Copyright (C) 2015 Peter Ivanov <ivanovp@gmail.com>
**
****************************************************************************/

#ifndef CONSOLESETTINGSDIALOG_H
#define CONSOLESETTINGSDIALOG_H

#include <QDialog>

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

    int getWaitAfterSendByte();
    void setWaitAfterSendByte(const int hexWrap);

private slots:
    void on_lineEndingTxComboBox_currentIndexChanged(int index);
    void on_lineEndingRxComboBox_currentIndexChanged(int index);
    QString hexString2Bin(const QString& hexString);
    QString bin2hexString(const QString& binString);

private:
    Ui::ConsoleSettingsDialog *ui;
};

#endif // CONSOLESETTINGSDIALOG_H
