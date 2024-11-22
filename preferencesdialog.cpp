#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "custompreferences.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    set(CustomPreferences::load());

    connect(this, &QDialog::accepted, this, [this](){
        CustomPreferences::setupApp(get());
    });
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void PreferencesDialog::set(QJsonObject preferences)
{
    auto style = preferences["style"].toObject();
    ui->styleTab->set(style);

    auto imagePreferences = preferences["image"].toObject();
    ui->imageTab->set(imagePreferences);
}

QJsonObject PreferencesDialog::get()
{
    QJsonObject preferences;
    preferences["style"] = ui->styleTab->get();
    preferences["image"] = ui->imageTab->get();
    return preferences;
}


void PreferencesDialog::on_pushButton_clicked()
{
    CustomPreferences::save(get());
}

