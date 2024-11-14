#ifndef PREFERENCESTYLE_H
#define PREFERENCESTYLE_H

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

private:
    Ui::PreferenceStyle *ui;
};

#endif // PREFERENCESTYLE_H
