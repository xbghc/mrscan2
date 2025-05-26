#include "examtab.h"
#include "ui_examtab.h"

#include "appearanceconfig.h"
#include "configmanager.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "store.h"
#include "utils.h"

#include <QDir>
#include <QFontMetrics>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QScrollBar>
#include <QUuid>

#include <memory>

namespace {} // namespace

ExamTab::ExamTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::examtab),
    m_patientDialog(new PatientInfoDialog), m_examDialog(new ExamEditDialog) {

    ui->setupUi(this);
    updatePatientList(true);

    m_exams = ExamConfig::initialExams();
    
    m_timer.setInterval(500);

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Status" << "Time");
    
    // 配置表格大小调整策略 - 只调整宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    
    updateExamTable();

    // 设置所有信号连接
    setupConnections();
}

ExamTab::~ExamTab() {
}

void ExamTab::tick(){
    auto procRow = processingRow();
    if(procRow < 0 || procRow >= m_exams.size()){
        LOG_ERROR(QString("ERROR: Processing row = %1").arg(procRow));
        return;
    }

    auto procExam = m_exams.at(procRow);
    procExam.setEndTime();
    auto timeStr = utils::secondsToString(procExam.time());
    ui->tableWidget->setItem(procRow, 2, new QTableWidgetItem(timeStr));
}

void ExamTab::updatePatientList(bool reload) {
    if (reload) {
        m_patientMap.clear();
        for (const auto &p_raw : store::loadAllPatients()) {
            if (p_raw) {
                m_patientMap.insert(p_raw->id(), std::shared_ptr<IPatient>(p_raw));
            }
        }
    }

    ui->comboBox->clear();
    for (const auto &p_shared_ptr : m_patientMap) {
        if (p_shared_ptr) {
            QString label = QString("%1 - %2").arg(p_shared_ptr->id(), p_shared_ptr->name());
            ui->comboBox->addItem(label, p_shared_ptr->id());
        }
    }
}

QString ExamTab::currentPatientId() const {
    return ui->comboBox->currentData().toString();
}

const Exam &ExamTab::currentExam() const {
    auto curRow = ui->tableWidget->currentRow();
    if (curRow < 0 || curRow >= m_exams.size()) { 
        LOG_ERROR("ExamTab::currentExam: No Current Exam or index out of bounds");
        throw std::runtime_error("ExamTab::currentExam: No Current Exam or index out of bounds");
    }
    return m_exams[curRow];
}

void ExamTab::onScanStarted(QString id) {
    int row = processingRow();
    if (row < 0 || row >= m_exams.size()) {
        LOG_ERROR(QString("onScanStarted: Invalid processing row %1").arg(row));
        return;
    }
    ui->comboBox->setEnabled(false);
    m_exams[row].setStartTime();
    m_exams[row].setId(id);
    updateExamTable();
    m_timer.start();
}

void ExamTab::onScanStoped() {
    auto processingRow = this->processingRow();
    if (processingRow < 0 || processingRow >= m_exams.size()) {
        LOG_ERROR(QString("onScanStoped: Invalid processing row %1").arg(processingRow));
        return;
    }
    m_exams[processingRow].setEndTime();
    m_exams[processingRow].setStatus(Exam::Status::Ready);  
    onCurrentExamChanged();
    m_timer.stop();
}

const Exam &ExamTab::setResponse(IExamResponse *response) {
    int procRow = processingRow();
     if (procRow < 0 || procRow >= m_exams.size()) {
        LOG_ERROR(QString("setResponse: Invalid processing row %1").arg(procRow));
        throw std::runtime_error("setResponse: Invalid processing row index");
    }
    auto &exam = m_exams[procRow];

    exam.setResponse(response);
    exam.setEndTime();
    exam.setStatus(Exam::Status::Done);

    auto patient_it = m_patientMap.find(currentPatientId());
    if (patient_it != m_patientMap.end()) {
        exam.setPatient(patient_it.value().get());
    } else {
        LOG_ERROR(QString("Patient not found in map for ID: %1").arg(currentPatientId()));
    }
    updateExamTable();

    /// @note 这是判断scout的方式
    if (exam.request().name().toLower() == "scout") {
        LOG_INFO("Scout result received");
        m_examDialog->setScout(exam);
    }

    m_timer.stop();
    return exam;
}

void ExamTab::onEditPatientButtonClicked() {
    auto currentId = currentPatientId();
    auto patient_it = m_patientMap.find(currentId);

    if (patient_it != m_patientMap.end()) {
        m_patientDialog->setPatient(patient_it.value().get());
        m_patientDialog->setType(PatientInfoDialog::Type::Edit);
        m_patientDialog->setModal(true);
        m_patientDialog->exec();
    } else {
        LOG_WARNING(QString("Patient with ID %1 not found for editing.").arg(currentId));
    }
}

void ExamTab::onNewPatientButtonClicked() {
    m_patientDialog->clear();
    m_patientDialog->setType(PatientInfoDialog::Type::New);
    m_patientDialog->setModal(true);
    m_patientDialog->exec();
}

void ExamTab::onPatientDialogAccepted() {
    auto name = m_patientDialog->name();
    auto gender = m_patientDialog->gender();
    auto birthday = m_patientDialog->birthday();

    if (m_patientDialog->type() == PatientInfoDialog::Type::New) {
        addPatient(name, birthday, gender);
        return;
    }

    // m_patientDialog->type() == PatientInfoDialog::Type::Edit
    auto id = m_patientDialog->id();
    auto patient_it = m_patientMap.find(id);

    if (patient_it != m_patientMap.end()) {
        auto p_shared_ptr = patient_it.value();
        p_shared_ptr->setName(name);
        p_shared_ptr->setBirthday(birthday);
        p_shared_ptr->setGender(gender);
        store::savePatient(p_shared_ptr.get());
        updatePatientList(false);
        int index = ui->comboBox->findData(id);
        if (index != -1) {
            ui->comboBox->setCurrentIndex(index);
        }
        return;
    }
    LOG_ERROR(QString("Edit Failed: Can't find patient with id: %1 in map").arg(id));
}

void ExamTab::onDeletePatientButtonClicked() {
    if (QMessageBox::question(
            this, tr("Delete?"), tr("Confirm delete this patient?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }
    auto id = ui->comboBox->currentData().toString();
    store::deletePatient(id);
    if (m_patientMap.remove(id) > 0) {
        updatePatientList(false);
        LOG_INFO(QString("Patient with ID %1 deleted.").arg(id));
    } else {
        LOG_ERROR(QString("Remove failed. Can't find patient with id: %1 in map").arg(id));
    }
}

void ExamTab::onShiftUpButtonClicked() {
    int curRow = currentRow();
    swap(curRow, curRow - 1);
}

void ExamTab::onShiftDownButtonClicked() {
    int curRow = currentRow();
    swap(curRow, curRow + 1);
}

void ExamTab::onRemoveExamButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_WARNING("No exam selected");
        return;
    }
    if (curRow >= 0 && curRow < m_exams.size()){ 
        m_exams.removeAt(curRow);
        updateExamTable();
    } else {
        LOG_WARNING(QString("Cannot remove exam, invalid row: %1").arg(curRow));
    }
}

void ExamTab::onCopyExamButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_WARNING("No exam selected");
        return;
    }
     if (curRow >= 0 && curRow < m_exams.size()){ 
        Exam newExam; 
        newExam.setRequest(m_exams[curRow].request()); 
        m_exams.insert(curRow + 1, newExam); 
        updateExamTable();
    } else {
        LOG_WARNING(QString("Cannot copy exam, invalid row: %1").arg(curRow));
    }
}

void ExamTab::onEditExamButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_INFO("Edit button: No exam selected");
        return;
    }
    if (curRow >= 0 && curRow < m_exams.size()){ 
        m_examDialog->setData(m_exams[curRow]);
        m_examDialog->setModal(true);
        m_examDialog->exec();
    } else {
         LOG_WARNING(QString("Cannot edit exam, invalid row: %1").arg(curRow));
    }
}

void ExamTab::onScanStopButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_WARNING("No exam selected");
        return;
    }

    // stop
    if (processingRow() == curRow) {
        auto id = m_exams[curRow].id();
        emit stopButtonClicked(id);
        return;
    }

    /**
   * @brief start
   * @details Flow:
   * startButtonClicked->scanner::scan->scanner::started
   */
    m_exams[curRow].setStatus(Exam::Status::Processing);

    auto request = m_exams[curRow].request();
    emit startButtonClicked(request);
}

void ExamTab::onExamDialogAccept() {
    QJsonObject parameters = m_examDialog->getParameters();
    auto curRow = currentRow();
    auto request = this->m_exams[curRow].request();
    request.setParams(parameters);
    this->m_exams[curRow].setRequest(request);
}

void ExamTab::onCurrentExamChanged() {
    int curRow = currentRow();

    if (curRow < 0) {
        ui->editExamButton->setEnabled(false);
        ui->scanButton->setEnabled(false);
        return;
    }

    ui->editExamButton->setEnabled(true);
    switch (m_exams[curRow].status()) {
    case Exam::Status::Ready:
        ui->scanButton->setText(tr("start"));
        ui->scanButton->setEnabled(true);
        break;
    case Exam::Status::Processing:
        ui->scanButton->setText(tr("stop"));
        ui->scanButton->setEnabled(true);
        break;
    case Exam::Status::Done:
        ui->scanButton->setText(tr("start"));
        ui->scanButton->setEnabled(false);
        break;
    }
}

int ExamTab::currentRow() const { return ui->tableWidget->currentRow(); }

void ExamTab::updateExamTable() {
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(m_exams.size());
    for (int i = 0; i < m_exams.size(); ++i) {
        auto exam = m_exams[i];
        
        auto nameItem = new QTableWidgetItem(exam.request().name());
        nameItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 0, nameItem);
        
        auto statusItem = new QTableWidgetItem(exam.statusString());
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 1, statusItem);

        auto seconds = exam.time();
        auto timeStr = utils::secondsToString(seconds);
        auto timeItem = new QTableWidgetItem(timeStr);
        timeItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 2, timeItem);
    }

    resizeTableToContents();
}

IPatient *ExamTab::getPatient(QString id) {
    auto patient_it = m_patientMap.find(id);
    if (patient_it != m_patientMap.end()) {
        return patient_it.value().get();
    }
    return nullptr;
}

void ExamTab::addPatient(QString name, QDate birthday,
                         IPatient::Gender gender) {
    QString newId = generateNewPatientId();

    IPatient* patient_raw = store::createNewPatient(newId, name, birthday, gender);
    if (!patient_raw) {
        LOG_ERROR("Failed to create new patient object via store.");
        return;
    }
    store::addPatient(patient_raw);

    std::shared_ptr<IPatient> patient_shared_ptr(patient_raw);
    m_patientMap.insert(newId, patient_shared_ptr);

    updatePatientList(false);

    int index = ui->comboBox->findData(newId);
    if (index != -1) {
        ui->comboBox->setCurrentIndex(index);
    }
    LOG_INFO(QString("New patient added, ID: %1").arg(newId));
}

QString ExamTab::generateNewPatientId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ExamTab::swap(int row1, int row2) {
    if (row1 < 0 || row1 >= m_exams.size() || row2 < 0 ||
        row2 >= m_exams.size()) {
        return;
    }
    m_exams.swapItemsAt(row1, row2);
    updateExamTable();
    ui->tableWidget->selectRow(row2);
}

int ExamTab::processingRow() const {
    for (int i = 0; i < m_exams.size(); i++) {
        if (m_exams.at(i).status() == Exam::Status::Processing) {
            return i;
        }
    }

    return -1;
}

void ExamTab::setupConnections() {
    // 定时器连接
    connect(&m_timer, &QTimer::timeout, this, &ExamTab::tick);
    
    // selected index changed
    connect(ui->tableWidget->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &ExamTab::onCurrentExamChanged);

    connect(ui->editPatientButton, &QToolButton::clicked, this,
            &ExamTab::onEditPatientButtonClicked);
    connect(ui->newPatientButton, &QToolButton::clicked, this,
            &ExamTab::onNewPatientButtonClicked);
    connect(m_patientDialog.get(), &PatientInfoDialog::accepted, this,
            &ExamTab::onPatientDialogAccepted);

    connect(ui->deletePatientButton, &QToolButton::clicked, this,
            &ExamTab::onDeletePatientButtonClicked);

    connect(ui->shiftUpButton, &QPushButton::clicked, this,
            &ExamTab::onShiftUpButtonClicked);
    connect(ui->shiftDownButton, &QPushButton::clicked, this,
            &ExamTab::onShiftDownButtonClicked);
    connect(ui->removeExamButton, &QPushButton::clicked, this,
            &ExamTab::onRemoveExamButtonClicked);
    connect(ui->copyButton, &QPushButton::clicked, this,
            &ExamTab::onCopyExamButtonClicked);
    connect(ui->editExamButton, &QPushButton::clicked, this,
            &ExamTab::onEditExamButtonClicked);
    connect(ui->scanButton, &QPushButton::clicked, this,
            &ExamTab::onScanStopButtonClicked);

    connect(m_examDialog.get(), &ExamEditDialog::accepted, this,
            &ExamTab::onExamDialogAccept);

    // 连接全局字体变化信号
    connect(config::Appearance::instance(), &config::Appearance::fontChanged,
            this, &ExamTab::resizeTableToContents);
}

void ExamTab::resizeTableToContents() {
    // 只调整列宽以适应内容
    ui->tableWidget->resizeColumnsToContents();
    
    // 计算表格需要的最小宽度
    int totalWidth = 0;
    for (int i = 0; i < ui->tableWidget->columnCount(); ++i) {
        totalWidth += ui->tableWidget->columnWidth(i);
    }
    
    // 添加边距
    totalWidth += ui->tableWidget->frameWidth() * 2;
    
    // 如果有垂直滚动条，添加滚动条宽度
    if (ui->tableWidget->verticalScrollBar()->isVisible()) {
        totalWidth += ui->tableWidget->verticalScrollBar()->width();
    }
}

QSize ExamTab::sizeHint() const {
    // 计算表格所需宽度
    int tableWidth = 0;
    for (int i = 0; i < ui->tableWidget->columnCount(); ++i) {
        tableWidth += ui->tableWidget->columnWidth(i);
    }
    
    // 添加表格边距
    tableWidth += ui->tableWidget->frameWidth() * 2;
    
    // 添加滚动条宽度（预估）
    tableWidth += ui->tableWidget->verticalScrollBar()->sizeHint().width();
    
    // 计算按钮区域宽度
    int buttonWidth = ui->buttonContainer->sizeHint().width();
    
    // 计算布局间距
    int layoutSpacing = ui->horizontalLayout_3->spacing();
    
    // 总宽度 = 表格宽度 + 按钮宽度 + 布局间距
    int totalWidth = tableWidth + buttonWidth + layoutSpacing;
    
    // 计算高度：头部 + 表格内容 + 布局边距
    int headerHeight = ui->header->sizeHint().height();
    int tableHeight = ui->tableWidget->horizontalHeader()->height() + 
                     (ui->tableWidget->rowCount() * ui->tableWidget->rowHeight(0)) + 
                     ui->tableWidget->frameWidth() * 2;
    
    // 限制表格最小高度
    tableHeight = qMax(150, tableHeight);
    
    int layoutMargins = ui->verticalLayout_2->contentsMargins().top() + 
                       ui->verticalLayout_2->contentsMargins().bottom() +
                       ui->verticalLayout_2->spacing();
    
    int totalHeight = headerHeight + tableHeight + layoutMargins;
    
    return QSize(totalWidth, totalHeight);
}

QSize ExamTab::minimumSizeHint() const {
    // 计算最小表格宽度（基于列标题）
    int minTableWidth = 0;
    QFontMetrics fm(ui->tableWidget->font());
    
    // 计算每列的最小宽度（基于列标题文本）
    QStringList headers = {"Name", "Status", "Time"};
    for (const QString &header : headers) {
        minTableWidth += fm.horizontalAdvance(header) + 20; // 添加一些边距
    }
    
    // 添加表格边距和滚动条宽度
    minTableWidth += ui->tableWidget->frameWidth() * 2;
    minTableWidth += ui->tableWidget->verticalScrollBar()->sizeHint().width();
    
    // 计算按钮区域的最小宽度
    int minButtonWidth = ui->buttonContainer->minimumSizeHint().width();
    
    // 计算布局间距
    int layoutSpacing = ui->horizontalLayout_3->spacing();
    
    // 总的最小宽度
    int minTotalWidth = minTableWidth + minButtonWidth + layoutSpacing;
    
    // 计算最小高度：头部 + 最小表格高度 + 布局边距
    int headerHeight = ui->header->minimumSizeHint().height();
    int minTableHeight = ui->tableWidget->horizontalHeader()->height() + 
                        ui->tableWidget->rowHeight(0) * 2 + // 至少显示2行
                        ui->tableWidget->frameWidth() * 2;
    
    int layoutMargins = ui->verticalLayout_2->contentsMargins().top() + 
                       ui->verticalLayout_2->contentsMargins().bottom() +
                       ui->verticalLayout_2->spacing();
    
    int minTotalHeight = headerHeight + minTableHeight + layoutMargins;
    
    return QSize(minTotalWidth, minTotalHeight);
}
