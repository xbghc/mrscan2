#include "historytab.h"
#include "ui_historytab.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryTab)
{
    ui->setupUi(this);

    m_model = new HistoryModel;
    ui->tableView->setModel(m_model);
    ui->tableView->resizeColumnsToContents();
}

HistoryTab::~HistoryTab()
{
    delete ui;
    delete m_model;
}

void HistoryTab::loadHistoryList()
{
    m_model->loadHistoryList();
}
