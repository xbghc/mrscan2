#ifndef PREFERENCEHISTORY_H
#define PREFERENCEHISTORY_H

#include "abstractpreferencestab.h"

namespace Ui {
class PreferenceHistory;
}

class PreferenceHistory : public AbstractPreferencesTab
{
    Q_OBJECT

public:
    explicit PreferenceHistory(QWidget *parent = nullptr);
    ~PreferenceHistory();

    void set(QJsonObject pref) override;
    QJsonObject get() override;

private:
    Ui::PreferenceHistory *ui;
};

#endif // PREFERENCEHISTORY_H 