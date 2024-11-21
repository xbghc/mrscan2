#include "historytab.h"
#include "ui_historytab.h"
#include "historytablemodel.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryTab)
{
    ui->setupUi(this);

    ui->tableView->setModel(new HistoryTableModel);
    ui->tableView->resizeColumnsToContents();
}

HistoryTab::~HistoryTab()
{
    delete ui;
}
