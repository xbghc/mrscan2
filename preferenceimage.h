#ifndef PREFERENCEIMAGE_H
#define PREFERENCEIMAGE_H

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

private:
    Ui::PreferenceImage *ui;
};

#endif // PREFERENCEIMAGE_H
