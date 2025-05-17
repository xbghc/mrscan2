#include "examtab.h"
#include "configmanager.h"
#include "exameditdialog.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "store.h"
#include "ui_examtab.h"
#include "utils.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QVector3D>
#include <memory>

namespace {} // namespace

ExamTab::ExamTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::examtab),
    m_patientDialog(new PatientInfoDialog) {

    ui->setupUi(this);
    updatePatientList(true);

    m_exams = ExamConfig::initialExams();
    updateExamTable();

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
}

ExamTab::~ExamTab() {
}

void ExamTab::updatePatientList(bool reload) {
    if (reload) {
        m_patients.clear();
        for(const auto &p:store::loadAllPatients()){
            m_patients.push_back(std::shared_ptr<IPatient>(p));
        }
    }

    ui->comboBox->clear();
    for (const auto& p:m_patients) {
        QString label = QString("%1 - %2").arg(p->id(), p->name());
        ui->comboBox->addItem(label, p->id());
    }
}

QString ExamTab::currentPatientId() const {
    return ui->comboBox->currentData().toString();
}

const Exam &ExamTab::currentExam() const {
    auto curRow = ui->tableWidget->currentRow();
    if (curRow >= 0) {
        throw std::runtime_error("ExamTab::getCurrentExam: No Current Exam");
    }

    return m_exams[curRow];
}

void ExamTab::onScanStarted(QString id) {
    int row = processingRow();

    ui->comboBox->setEnabled(false); // 扫描开始后不能重新选择病人

    m_exams[row].setStartTime();
    m_exams[row].setId(id);

    updateExamTable();
}

void ExamTab::onScanStoped() {
    ui->scanButton->setText(tr("start"));
    ui->scanButton->setEnabled(true);

    ui->comboBox->setEnabled(true);
}

const Exam &ExamTab::setResponse(IExamResponse *response) {
    auto& exam = m_exams[processingRow()];
    exam.setResponse(response);
    exam.setEndTime();
    exam.setStatus(Exam::Status::Done);

    auto patient = getPatient(currentPatientId());
    exam.setPatient(patient);

    updateExamTable();
    return exam;
}

void ExamTab::onEditPatientButtonClicked() {
    const auto& patient = m_patients[ui->comboBox->currentIndex()];

    m_patientDialog->setPatient(patient.get());
    m_patientDialog->setType(PatientInfoDialog::Type::Edit);

    m_patientDialog->setModal(true);
    m_patientDialog->exec();
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

    for (const auto& p:m_patients) {
        if (p->id() == id) {
            p->setName(name);
            p->setBirthday(birthday);
            p->setGender(gender);
            store::savePatient(p.get());
            updatePatientList();
            return;
        }
    }

    LOG_ERROR(QString("Edit Failed: Can't find patient with id: %1").arg(id));
}

void ExamTab::onDeletePatientButtonClicked() {
    if (QMessageBox::question(
            this, tr("Delete?"), tr("Confirm delete this patient?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    auto id = ui->comboBox->currentData().toString();
    store::deletePatient(id);
    for (int i = 0; i < m_patients.size(); i++) {
        if (m_patients[i]->id() == id) {
            m_patients.removeAt(i);
            updatePatientList();
            return;
        }
    }

    LOG_ERROR(QString("Remove failed. Can't find patient with id: %1").arg(id));
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

    m_exams.removeAt(curRow);
}

void ExamTab::onCopyExamButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_WARNING("No exam selected");
        return;
    }

    m_exams.insert(curRow + 1, Exam());
    m_exams[curRow + 1].setRequest(m_exams[curRow].request());
}

void ExamTab::onEditExamButtonClicked() {
    int curRow = currentRow();
    if (curRow == -1) {
        LOG_WARNING("No exam selected");
        return;
    }

    ExamEditDialog dlg(this);
    dlg.setData(m_exams[curRow]);
    connect(&dlg, &QDialog::accepted, this, [&]() {
        QJsonObject parameters = dlg.getParameters();
        auto request = this->m_exams[curRow].request();
        request.setParams(parameters);
        this->m_exams[curRow].setRequest(request);
    });

    // TODO The readability of examModel code is poor, after adjustment, determine
    // whether to scan scout, if so, call dlg.setScoutImages Below is mock data
    QList<QImage> images;
    QList<QVector3D> angles;
    QList<QVector3D> offsets;
    for (int i = 0; i < 9; i++) {
        images.push_back(QImage(256, 256, QImage::Format_Grayscale8));
        angles.push_back(QVector3D(90, 0, 0));
        offsets.push_back(QVector3D(0, -40 + 10 * i, 0));
    }
    dlg.setScoutImages(images, 256, angles, offsets);

    dlg.setModal(true);
    dlg.exec();
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
   * @details 流程为：
   * startButtonClicked->scanner::scan->scanner::started
   */
    m_exams[curRow].setStatus(Exam::Status::Processing);

    auto request = m_exams[curRow].request();
    emit startButtonClicked(request);
}

void ExamTab::onCurrentExamChanged() {
    int curRow = currentRow();

    if (curRow < 0) {
        return;
    }

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

IPatient* ExamTab::getPatient(QString id) {
    for (const auto& p : m_patients) {
        if (p->id() == id) {
            return p.get();
        }
    }

    LOG_ERROR(QString("Can't find patient with id: %1").arg(id));
    return nullptr;
}

void ExamTab::addPatient(QString name, QDate birthday,
                         IPatient::Gender gender) {
    QString id = QString::number(nextPatientId());

    auto patient = new JsonPatient();
    patient->setId(id);
    patient->setName(name);
    patient->setBirthday(birthday);
    patient->setGender(gender);
    m_patients.push_back(std::shared_ptr<IPatient>(patient));

    setNextId(id.toInt() + 1);
    store::addPatient(patient);
}

int ExamTab::nextPatientId() {

    const static QString kFilePath = "./patients/nextId";
    auto kDirPath = "./patients";
    QDir dir(kDirPath);
    if (!dir.exists() && dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << kDirPath;
        return -1;
    }

    QFile file(kFilePath);
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to open file: " << kFilePath;
            return -1;
        } else {
            file.write("1");
            return 0;
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "failed to open file: " << kFilePath;
        return -1;
    }
    return file.readAll().toInt();
}

void ExamTab::setNextId(int id) {
    const static QString kFilePath = "./patients/nextId";
    QFile file(kFilePath);

    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("failed to write to: %1").arg(kFilePath));
        return;
    }

    file.write(QByteArray::number(id));
    return;
}

void ExamTab::swap(int row1, int row2) {
    if (row1 < 0 || row2 < 0 || row1 >= m_exams.size() ||
        row2 >= m_exams.size()) {
        LOG_WARNING(QString("Can't swap between row %1 and %2").arg(row1, row2));
        return;
    }

    m_exams.swapItemsAt(row1, row2);
    updateExamTable();
}

void ExamTab::updateExamTable() {
    auto row = ui->tableWidget->currentRow();

    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(m_exams.size());

    QStringList headers;
    headers << "Name" << "Time" << "Status";

    for (int i = 0; i < m_exams.size(); i++) {
        const auto &exam = m_exams[i];
        const auto &request = exam.request();

        auto name = request.name();
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(name));

        auto time = exam.time();
        auto timeStr =
            QString("%1:%2").arg(time / 60).arg(time % 60, 2, 10, 0, QChar(u'0'));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(timeStr));

        auto status = exam.status();
        QString statusStr;
        switch (status) {
        case Exam::Status::Ready:
            statusStr = "Ready";
            break;
        case Exam::Status::Processing:
            statusStr = "Processing";
            break;
        case Exam::Status::Done:
            statusStr = "Done";
            break;
        }
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(statusStr));
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    if (row >= 0 && row < ui->tableWidget->rowCount()) {
        ui->tableWidget->selectRow(row);
    }
}

int ExamTab::currentRow() const { return ui->tableWidget->currentRow(); }

int ExamTab::processingRow() const {
    for (int i = 0; i < m_exams.size(); i++) {
        const auto &exam = m_exams[i];
        if (exam.status() == Exam::Status::Processing) {
            return i;
        }
    }

    return -1;
}
