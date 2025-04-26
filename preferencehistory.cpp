#include "preferencehistory.h"
#include "ui_preferencehistory.h"
#include "pathmanager.h"

PreferenceHistory::PreferenceHistory(QWidget *parent) :
    AbstractPreferencesTab(parent),
    ui(new Ui::PreferenceHistory)
{
    ui->setupUi(this);
    
    ui->lineEditFormat->setText(PathManager::getHistoryPathFormat());
}

PreferenceHistory::~PreferenceHistory()
{
}

void PreferenceHistory::set(QJsonObject pref)
{
    if (pref.contains("pathFormat")) {
        ui->lineEditFormat->setText(pref["pathFormat"].toString());
    }
}

QJsonObject PreferenceHistory::get()
{
    QJsonObject obj;
    obj["pathFormat"] = ui->lineEditFormat->text();
    return obj;
} 