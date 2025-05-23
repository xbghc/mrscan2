#include "historytab.h"
#include "ui_historytab.h"
#include "appearanceconfig.h"

#include "store.h"
#include "utils.h"

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryTab) {
    ui->setupUi(this);

    m_model = new HistoryModel;
    ui->tableView->setModel(m_model);
    ui->tableView->resizeColumnsToContents();

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置所有信号连接
    setupConnections();
}

HistoryTab::~HistoryTab() {
    delete ui;
    delete m_model;
}

void HistoryTab::setupConnections() {
    // 连接tableView选择变化信号
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &HistoryTab::onCurrentRowChanged);

    // 连接全局字体变化信号
    connect(Config::Appearance::instance(), &Config::Appearance::fontChanged,
            ui->tableView, &QTableView::resizeColumnsToContents);
}

void HistoryTab::addExamToView(const Exam &exam)
{
    if (exam.id().isEmpty() || !exam.patient() || exam.patient()->id().isEmpty()) {
        LOG_WARNING("Attempted to add an exam with missing ID or patient information to history view.");
        return;
    }
    m_model->addExam(exam.id(), exam.patient()->id(), exam.startTime());
    // Add to cache as well, so if it's immediately selected, it's available
    m_cache[std::pair(exam.patient()->id(), exam.id())] = exam;
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
