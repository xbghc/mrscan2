#include "historytab.h"
#include "ui_historytab.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryTab)
{
    ui->setupUi(this);

    m_model = new HistoryTableModel;
    ui->tableView->setModel(m_model);
    ui->tableView->resizeColumnsToContents();

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous){
        int curRow = current.row();
        emit currentHistoryChanged(m_model->getHistoryObj(curRow));
    });
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
