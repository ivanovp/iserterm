#include "consolesettingsdialog.h"
#include "ui_consolesettingsdialog.h"

ConsoleSettingsDialog::ConsoleSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleSettingsDialog)
{
    ui->setupUi(this);
}

ConsoleSettingsDialog::~ConsoleSettingsDialog()
{
    delete ui;
}
