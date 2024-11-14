#include "tuningradiofrequencypower.h"
#include "ui_tuningradiofrequencypower.h"

TuningRadioFrequencyPower::TuningRadioFrequencyPower(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TuningRadioFrequencyPower)
{
    ui->setupUi(this);
}

TuningRadioFrequencyPower::~TuningRadioFrequencyPower()
{
    delete ui;
}
