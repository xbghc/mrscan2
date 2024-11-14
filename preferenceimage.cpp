#include "preferenceimage.h"
#include "ui_preferenceimage.h"

PreferenceImage::PreferenceImage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferenceImage)
{
    ui->setupUi(this);
}

PreferenceImage::~PreferenceImage()
{
    delete ui;
}
