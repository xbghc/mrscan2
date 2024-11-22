#ifndef PREFERENCEIMAGE_H
#define PREFERENCEIMAGE_H

#include <QJsonObject>
#include <QWidget>

namespace Ui {
class PreferenceImage;
}

class PreferenceImage : public QWidget
{
    Q_OBJECT

public:
    explicit PreferenceImage(QWidget *parent = nullptr);
    ~PreferenceImage();

    void set(QJsonObject imagePreferences);
    QJsonObject get();

private:
    Ui::PreferenceImage *ui;
};

#endif // PREFERENCEIMAGE_H
