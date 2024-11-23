#ifndef PREFERENCESTYLE_H
#define PREFERENCESTYLE_H

#include <QJsonObject>
#include <QStyleHints>
#include <QWidget>

namespace Ui {
class PreferenceStyle;
}

class PreferenceStyle : public QWidget
{
    Q_OBJECT

public:
    explicit PreferenceStyle(QWidget *parent = nullptr);
    ~PreferenceStyle();

    void set(QJsonObject style);
    QJsonObject get();

private:
    Ui::PreferenceStyle *ui;

    void setFontSize(int size);
    void setFontFamily(QString family);
    void setTheme(QString theme);
    void setColorScheme(Qt::ColorScheme color);
};

#endif // PREFERENCESTYLE_H
