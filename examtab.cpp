#include "examtab.h"
#include "exameditdialog.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "ui_examtab.h"
#include "examhistory.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QVector3D>

namespace {} // namespace

ExamTab::ExamTab(QWidget *parent) : QWidget(parent), ui(new Ui::studytab) {
    ui->setupUi(this);

    loadPatients();

    examModel = new ExamModel;
    ui->tableView->setModel(examModel);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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

    connect(ui->editPatientButton, &QToolButton::clicked, this, &ExamTab::openEditPatientDialog);
    connect(ui->newPatientButton, &QToolButton::clicked, this, &ExamTab::openNewPatientDialog);
    connect(ui->deletePatientButton, &QToolButton::clicked, this, &ExamTab::deletePatient);

    connect(ui->shiftUpButton, &QPushButton::clicked, this, &ExamTab::shiftUp);
    connect(ui->shiftDownButton, &QPushButton::clicked, this, &ExamTab::shiftDown);
    connect(ui->removeExamButton, &QPushButton::clicked, this, &ExamTab::removeExam);
    connect(ui->copyButton, &QPushButton::clicked, this, &ExamTab::copyExam);
    connect(ui->editExamButton, &QPushButton::clicked, this, &ExamTab::editExam);
    connect(ui->scanButton, &QPushButton::clicked, this, &ExamTab::onScanButtonClicked);
}

ExamTab::~ExamTab() {
    delete ui;
    delete examModel;
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

void ExamTab::onScanStarted(int id)
{
    int curRow = currentExamIndex();
    if(id < 0){
        qDebug() << "scan failed";
        return;
    }

    ui->scanButton->setText(tr("stop"));
    ui->comboBox->setEnabled(false);
    examModel->examStarted(curRow, id);
}

void ExamTab::onScanEnd(QByteArray response) // 以后会移除参数，Exam和数据无关
{
    int patientId = this->ui->comboBox->currentData().toInt();

    QJsonObject request = examModel->getExamData(examModel->getScanningRow());
    ExamHistory history(request, response);
    history.setPatient(patientId);
    history.save();

    emit fileSaved(history);

    // TODO 将上方所有内容移动到mainWindow中

    this->ui->comboBox->setEnabled(true);
    this->ui->scanButton->setText("start");
    this->ui->scanButton->setEnabled(false);
    examModel->examDone();

}

void ExamTab::openEditPatientDialog()
{
    PatientInfoDialog dialog(this);
    int id = ui->comboBox->currentData().toInt();
    Patient patient = Patient::getPatient(id);
    dialog.setPatient(&patient);

    dialog.setModal(true);
    connect(&dialog, &PatientInfoDialog::accepted, this,
            [this]() { loadPatients(); });
    dialog.exec();
}

void ExamTab::openNewPatientDialog() {
    // button clicked to create new patient
    PatientInfoDialog dialog(this);
    dialog.setPatient(nullptr);
    dialog.setModal(true);

    connect(&dialog, &PatientInfoDialog::accepted, this,
            [this]() { loadPatients(); });
    dialog.exec();
}

void ExamTab::deletePatient() {
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

void ExamTab::shiftUp() {

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
void ExamTab::shiftDown() {

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

void ExamTab::removeExam() {

    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    examModel->removeRow(curRow);
}

// copy button clicked
void ExamTab::copyExam() {
    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    examModel->copyRow(curRow);
}

// edit button clicked
void ExamTab::editExam() {
    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    ExamEditDialog dlg(this);
    QJsonObject data = examModel->getExamData(curRow);
    dlg.setData(data);
    connect(&dlg, &QDialog::accepted, this, [&]() {
        QJsonObject parameters = dlg.getParameters();
        this->examModel->setExamParams(curRow, parameters);
    });

    // TODO examModel的代码可读性太差，调整好之后，判断是否扫描scout，如果是，调用dlg.setScoutImages
    // 下面是mock的数据
    QList<QImage> images;
    QList<QVector3D> angles;
    QList<QVector3D> offsets;
    for(int i=0;i<9;i++){
        images.push_back(QImage(256, 256, QImage::Format_Grayscale8));
        angles.push_back(QVector3D(90, 0, 0));
        offsets.push_back(QVector3D(0, -40+10*i, 0));
    }
    dlg.setScoutImages(images, 256, angles, offsets);

    dlg.setModal(true);
    dlg.exec();
}

// start/stop button clicked
void ExamTab::onScanButtonClicked()
{
    int curRow = currentExamIndex();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    // stop
    if(examModel->getScanningRow() == curRow){
        int id = examModel->getScanningId();

        emit onStopButtonClicked(id);

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
    emit onStartButtonClicked(exam);
}

QString ExamTab::getStatus(int row)
{
    QModelIndex index = examModel->index(row, 2);
    return examModel->data(index).toString();
}

