#include "tuningshimming.h"
#include "ui_tuningshimming.h"

TuningShimming::TuningShimming(QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::TuningShimming>())
{
    ui->setupUi(this);
}

TuningShimming::~TuningShimming() = default;
