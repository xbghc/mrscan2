#ifndef TUNINGCENTRALFREQUENCY_H
#define TUNINGCENTRALFREQUENCY_H

#include <QDialog>

namespace Ui {
class TuningCentralFrequency;
}

class TuningCentralFrequency : public QDialog
{
    Q_OBJECT

public:
    explicit TuningCentralFrequency(QWidget *parent = nullptr);
    ~TuningCentralFrequency();

private:
    Ui::TuningCentralFrequency *ui;
};

#endif // TUNINGCENTRALFREQUENCY_H
