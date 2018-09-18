#include "finddialog.h"
#include "ui_finddialog.h"

#include <QSettings>

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

  ui->findComboBox->setCurrentText(findStr);
  ui->caseSensCheckBox->setChecked(caseSens);
  ui->wholeWordsCheckBox->setChecked(wholeWords);
  ui->regExCheckBox->setChecked(regEx);
}

FindDialog::~FindDialog()
{
  QSettings settings;

  settings.setValue("find/text", ui->findComboBox->currentText());
  settings.setValue("find/caseSens", ui->caseSensCheckBox->isChecked());
  settings.setValue("find/wholeWords", ui->wholeWordsCheckBox->isChecked());
  settings.setValue("find/regEx", ui->regExCheckBox->isChecked());

  delete ui;
}
