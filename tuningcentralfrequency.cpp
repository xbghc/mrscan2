#include "tuningcentralfrequency.h"
#include "ui_tuningcentralfrequency.h"

TuningCentralFrequency::TuningCentralFrequency(QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::TuningCentralFrequency>())
{
    ui->setupUi(this);
}

TuningCentralFrequency::~TuningCentralFrequency() = default;
