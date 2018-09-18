#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <ui_finddialog.h>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

    QString getText() { return ui->findComboBox->currentText(); }
    bool isCaseSens() { return ui->caseSensCheckBox->isChecked(); }
    bool isWholeWords() { return ui->wholeWordsCheckBox->isChecked(); }
    bool isRegEx() { return ui->regExCheckBox->isChecked(); }

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
