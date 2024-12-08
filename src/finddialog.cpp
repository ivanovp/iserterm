#include "finddialog.h"
#include "ui_finddialog.h"

#include <QSettings>
#include <QLineEdit>

FindDialog::FindDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FindDialog)
{
  ui->setupUi(this);

  QSettings settings;

  QString findStr = settings.value("find/text", "").toString();
  bool caseSens = settings.value("find/caseSens", false).toBool();
  bool wholeWords = settings.value("find/wholeWords", false).toBool();
  bool regEx = settings.value("find/regEx", false).toBool();

  ui->findComboBox->addItems(loadHistory(Multistring::ASCII));
  ui->findComboBox->setCurrentText(findStr);
  ui->findComboBox->lineEdit()->selectAll();
  ui->caseSensCheckBox->setChecked(caseSens);
  ui->wholeWordsCheckBox->setChecked(wholeWords);
  ui->regExCheckBox->setChecked(regEx);
}

/**
 * @brief FindDialog::loadHistory Load history of sendLineEdit values
 * @param mode ASCII, hexadecimal, etc. @see Multistring::mode_t
 * @return Loaded history.
 */
QStringList FindDialog::loadHistory(Multistring::mode_t mode)
{
    QSettings settings;
    int count = settings.value(QString("find/historyCount%1").arg(static_cast<int>(mode)), 0).toInt();
    QStringList history;

    for (int i = 0; i < count; i++)
    {
        QString s = settings.value(QString("find/history%1/%2").arg(static_cast<int>(mode)).arg(i), 0).toString();
        history.append(s);
    }

    return history;
}

/**
 * @brief FindDialog::saveHistory Save history of sendLineEdit values
 * @param mode ASCII, hexadecimal, etc. @see Multistring::mode_t
 * @param history History to be saved.
 */
void FindDialog::saveHistory(Multistring::mode_t mode, const QStringList &history)
{
    QSettings settings;
    int count = history.count();
    settings.setValue(QString("find/historyCount%1").arg(static_cast<int>(mode)), count);

    for (int i = 0; i < count; i++)
    {
        settings.setValue(QString("find/history%1/%2").arg(static_cast<int>(mode)).arg(i), history[i]);
    }
}

QStringList FindDialog::getCurrentHistory()
{
    QStringList history;

    for (int i = 0; i < ui->findComboBox->count(); i++)
    {
        history.append(ui->findComboBox->itemText(i));
    }

    return history;
}


FindDialog::~FindDialog()
{
  QSettings settings;

  settings.setValue("find/text", ui->findComboBox->currentText());
  settings.setValue("find/caseSens", ui->caseSensCheckBox->isChecked());
  settings.setValue("find/wholeWords", ui->wholeWordsCheckBox->isChecked());
  settings.setValue("find/regEx", ui->regExCheckBox->isChecked());

  saveHistory(Multistring::ASCII, getCurrentHistory());

  delete ui;
}
