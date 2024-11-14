#include "tuningshimming.h"
#include "ui_tuningshimming.h"

TuningShimming::TuningShimming(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TuningShimming)
{
    ui->setupUi(this);
}

TuningShimming::~TuningShimming()
{
    delete ui;
}
