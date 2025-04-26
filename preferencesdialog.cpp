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
}

void PreferencesDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void PreferencesDialog::set(QJsonObject preferences)
{
    for(int i=0;i<ui->stackedWidget->count();i++){
        auto tab = qobject_cast<AbstractPreferencesTab*>(ui->stackedWidget->widget(i));
        auto jsonKey = ui->listWidget->item(i)->text().toLower();
        tab->set(preferences[jsonKey].toObject());
    }
}

QJsonObject PreferencesDialog::get()
{
    QJsonObject preferences;
    for(int i=0;i<ui->stackedWidget->count();i++){
        auto tab = qobject_cast<AbstractPreferencesTab*>(ui->stackedWidget->widget(i));
        auto jsonKey = ui->listWidget->item(i)->text().toLower();
        preferences[jsonKey] = tab->get();
    }
    return preferences;
}


void PreferencesDialog::on_pushButton_clicked()
{
    CustomPreferences::save(get());
}

