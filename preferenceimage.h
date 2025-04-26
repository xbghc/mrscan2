#ifndef PREFERENCEIMAGE_H
#define PREFERENCEIMAGE_H

#include <QJsonObject>
#include <QWidget>
#include <memory>

#include "abstractpreferencestab.h"

namespace Ui {
class PreferenceImage;
}

class PreferenceImage : public AbstractPreferencesTab
{
    Q_OBJECT

public:
    explicit PreferenceImage(QWidget *parent = nullptr);
    ~PreferenceImage();

    void set(QJsonObject imagePreferences) override;
    QJsonObject get() override;

private:
    std::unique_ptr<Ui::PreferenceImage> ui;
};

#endif // PREFERENCEIMAGE_H
