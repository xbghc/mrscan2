#include "historytab.h"
#include "ui_historytab.h"

#include "store.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryTab) {
    ui->setupUi(this);

    m_model = new HistoryModel;
    ui->tableView->setModel(m_model);
    ui->tableView->resizeColumnsToContents();

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &HistoryTab::onCurrentRowChanged);
}

HistoryTab::~HistoryTab() {
    delete ui;
    delete m_model;
}

void HistoryTab::loadHistoryList() { m_model->loadHistoryList(); }

void HistoryTab::onCurrentRowChanged()
{
    auto row = ui->tableView->currentIndex().row();
    auto eid = this->m_model->data(this->m_model->index(row, 0)).toString();
    auto pid = m_model->data(m_model->index(row, 1)).toString();
    auto key = std::pair(pid, eid);
    if (!m_cache.contains(key)) {
        m_cache[key] = store::loadExam(pid, eid);
    }
    emit currentItemChanged(m_cache[key]);
}
