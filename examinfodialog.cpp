#include "examinfodialog.h"
#include "ui_examinfodialog.h"

ExamInfoDialog::ExamInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExamInfoDialog)
{
    ui->setupUi(this);
}

ExamInfoDialog::~ExamInfoDialog()
{
    delete ui;
}
