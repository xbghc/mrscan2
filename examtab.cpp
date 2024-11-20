#include "examtab.h"
#include "examinfodialog.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "ui_examtab.h"
#include "examhistory.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

namespace {} // namespace

ExamTab::ExamTab(QWidget *parent) : QWidget(parent), ui(new Ui::studytab) {
    ui->setupUi(this);

    loadPatients();

    examModel = new ExamTableModel;
    ui->tableView->setModel(examModel);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    adapter = new ScannerAdapter;
    adapter->open();

    // selected index changed
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous){
        int curRow = current.row();

        int scanningRow = examModel->getScanningRow();
        if(scanningRow == curRow){
            ui->scanButton->setText(tr("stop"));
            ui->scanButton->setEnabled(true);
            return;
        }

        ui->scanButton->setText(tr("start"));
        bool enable = (scanningRow == -1 && getStatus(curRow) == "Ready");
        ui->scanButton->setEnabled(enable);
    });

    // scanned
    connect(adapter, &ScannerAdapter::scanned, this, [this](QByteArray responseBytes){
        // 1. save to file in ./patients/patientID/scanID/
        int scanId;
        ScannerResponse response = ScannerResponse::fromBytes(responseBytes);
        if(response.getId() != examModel->getScanningId()){
            qDebug() << "scan id unmatched, expeted: " << examModel->getScanningId() << "response: " << response.getId();
            return;
        }

        int patientId = this->ui->comboBox->currentData().toInt();

        QJsonObject request = examModel->getExamData(examModel->getScanningRow());
        ExamHistory history(request, responseBytes);
        history.setPatient(patientId);
        history.save();

        // 2. change status and enable components
        this->ui->comboBox->setEnabled(true);
        this->ui->scanButton->setText("start");
        this->ui->scanButton->setEnabled(false);
        examModel->examDone();

        // 3. invoke the function to visualize the result
        emit displayExam(history);
    });
}

ExamTab::~ExamTab() {
    delete ui;
    delete examModel;

    adapter->close();
    delete adapter;
}

int ExamTab::currentExamIndex()
{
    return ui->tableView->currentIndex().row();
}

void ExamTab::loadPatients() {
    ui->comboBox->clear();
    Patient::loadPatients();
    for (auto p : Patient::patientsList) {
        QString label = QString::number(p.getId()) + "-" + p.getName();
        ui->comboBox->addItem(label, p.getId());
    }
}

void ExamTab::on_toolButton_3_clicked() {
    // button clicked to create new patient
    PatientInfoDialog dialog(this);
    dialog.setPatient(nullptr);
    dialog.setModal(true);

    connect(&dialog, &PatientInfoDialog::accepted, this,
            [this]() { loadPatients(); });
    dialog.exec();
}

void ExamTab::on_toolButton_clicked() {
    // button clicked to edit patient
    PatientInfoDialog dialog(this);
    int id = ui->comboBox->currentData().toInt();
    Patient patient = Patient::getPatient(id);
    dialog.setPatient(&patient);

    dialog.setModal(true);
    connect(&dialog, &PatientInfoDialog::accepted, this,
            [this]() { loadPatients(); });
    dialog.exec();
}

void ExamTab::on_toolButton_2_clicked() {
    // button clicked to remove patient
    if (QMessageBox::question(this, "delete?", "confim to delete?",
                              QMessageBox::Yes | QMessageBox::No) ==
        QMessageBox::Yes) {
        PatientInfoDialog dialog(this);
        int id = ui->comboBox->currentData().toInt();
        Patient::removePatient(id);
        loadPatients();
    }
}

// up button clicked
void ExamTab::on_pushButton_3_clicked() {

    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == 0) {
        qDebug() << "alreay the first exam";
        return;
    }

    examModel->swapRows(curRow, curRow - 1);
}

// down button clicked
void ExamTab::on_pushButton_4_clicked() {

    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == examModel->rowCount() - 1) {
        qDebug() << "already the last exam";
        return;
    }

    examModel->swapRows(curRow, curRow + 1);
}

// remove button clicked
void ExamTab::on_pushButton_5_clicked() {

    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    examModel->removeRow(curRow);
}

// copy button clicked
void ExamTab::on_pushButton_6_clicked() {
    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    examModel->copyRow(curRow);
}

// edit button clicked
void ExamTab::on_pushButton_2_clicked() {

    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    ExamInfoDialog dialog(this);
    QJsonObject data = examModel->getExamData(curRow);
    dialog.setData(data);
    connect(&dialog, &QDialog::accepted, this, [&]() {
        QJsonObject parameters = dialog.getParameters();
        this->examModel->setExamParams(curRow, parameters);
    });

    dialog.setModal(true);
    dialog.exec();
}

// start/stop button clicked
void ExamTab::on_scanButton_clicked()
{
    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    // stop
    if(examModel->getScanningRow() == curRow){
        int id = examModel->getScanningId();

        adapter->stop(id);
        int stopid = examModel->examStoped();
        if(stopid != id){
            qDebug() << "exam model stop a wrong id, expected id: " << id << ", actual id: " << stopid;
        }
        ui->scanButton->setText(tr("start"));
        this->ui->comboBox->setEnabled(true);

        return;
    }

    // start
    QJsonObject exam = examModel->getExamData(curRow);
    int id = adapter->scan(exam);
    if(id < 0){
        qDebug() << "scan failed";
        return;
    }

    ui->scanButton->setText(tr("stop"));
    ui->comboBox->setEnabled(false);
    examModel->examStarted(curRow, id);
}

QString ExamTab::getStatus(int row)
{
    QModelIndex index = examModel->index(row, 2);
    return examModel->data(index).toString();
}

