#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <ui_finddialog.h>
#include "multistring.h"

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
    void setHistory(QStringList &items) { ui->findComboBox->addItems(items); }
    bool isCaseSens() { return ui->caseSensCheckBox->isChecked(); }
    bool isWholeWords() { return ui->wholeWordsCheckBox->isChecked(); }
    bool isRegEx() { return ui->regExCheckBox->isChecked(); }

protected:
    QStringList loadHistory(Multistring::mode_t mode);
    void saveHistory(Multistring::mode_t mode, const QStringList &history);
    QStringList getCurrentHistory();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
