#include "tuningradiofrequencypower.h"
#include "ui_tuningradiofrequencypower.h"

TuningRadioFrequencyPower::TuningRadioFrequencyPower(QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::TuningRadioFrequencyPower>())
{
    ui->setupUi(this);
}

TuningRadioFrequencyPower::~TuningRadioFrequencyPower() = default;
