#include "tuningcentralfrequency.h"
#include "ui_tuningcentralfrequency.h"

TuningCentralFrequency::TuningCentralFrequency(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TuningCentralFrequency)
{
    ui->setupUi(this);
}

TuningCentralFrequency::~TuningCentralFrequency()
{
    delete ui;
}
