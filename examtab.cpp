#include "examtab.h"
#include "ui_examtab.h"

ExamTab::ExamTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::studytab)
{
    ui->setupUi(this);
}

ExamTab::~ExamTab()
{
    delete ui;
}
