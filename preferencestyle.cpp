#include "preferencestyle.h"
#include "ui_preferencestyle.h"

PreferenceStyle::PreferenceStyle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferenceStyle)
{
    ui->setupUi(this);
}

PreferenceStyle::~PreferenceStyle()
{
    delete ui;
}
