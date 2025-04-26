#ifndef PREFERENCESTYLE_H
#define PREFERENCESTYLE_H

#include <QJsonObject>
#include <QStyleHints>
#include <QWidget>
#include <memory>

#include "abstractpreferencestab.h"

namespace Ui {
class PreferenceStyle;
}

class PreferenceStyle : public AbstractPreferencesTab
{
    Q_OBJECT

public:
    explicit PreferenceStyle(QWidget *parent = nullptr);
    ~PreferenceStyle();

    void set(QJsonObject style) override;
    QJsonObject get() override;

private:
    std::unique_ptr<Ui::PreferenceStyle> ui;

    void setFontSize(int size);
    void setFontFamily(QString family);
    void setTheme(QString theme);
    void setColorScheme(Qt::ColorScheme color);
};

#endif // PREFERENCESTYLE_H
