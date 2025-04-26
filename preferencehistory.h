#ifndef PREFERENCEHISTORY_H
#define PREFERENCEHISTORY_H

#include "abstractpreferencestab.h"
#include <memory>

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
    std::unique_ptr<Ui::PreferenceHistory> ui;
};

#endif // PREFERENCEHISTORY_H 