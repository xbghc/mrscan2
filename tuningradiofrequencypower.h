#ifndef TUNINGRADIOFREQUENCYPOWER_H
#define TUNINGRADIOFREQUENCYPOWER_H

#include <QDialog>
#include <memory>

namespace Ui {
class TuningRadioFrequencyPower;
}

class TuningRadioFrequencyPower : public QDialog
{
    Q_OBJECT

public:
    explicit TuningRadioFrequencyPower(QWidget *parent = nullptr);
    ~TuningRadioFrequencyPower();

private:
    std::unique_ptr<Ui::TuningRadioFrequencyPower> ui;
};

#endif // TUNINGRADIOFREQUENCYPOWER_H
