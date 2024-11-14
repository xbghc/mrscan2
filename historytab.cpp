#include "historytab.h"
#include "ui_historytab.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryTab)
{
    ui->setupUi(this);
}

HistoryTab::~HistoryTab()
{
    delete ui;
}
