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

private:
    Ui::ConsoleSettingsDialog *ui;
};

#endif // CONSOLESETTINGSDIALOG_H
