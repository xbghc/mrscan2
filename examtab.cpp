#include "examtab.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "ui_examtab.h"
#include "examinfodialog.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

namespace {

void loadExams(Ui::studytab *ui) {
    const static QString kPath = "./configs/exams.json";

    QDir dir("./configs");
    if (!dir.exists() && dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << dir.path();
    }

    QFile file(kPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(nullptr, "Warning", "No Exam Configuration!");
        return;
    }

    ui->tableWidget->clearContents();

    QJsonArray exams = QJsonDocument::fromJson(file.readAll()).array();
    ui->tableWidget->setRowCount(exams.size());
    for (int i = 0; i < exams.size(); i++) {
        QJsonObject obj = exams[i].toObject();

        QTableWidgetItem *nameItem = new QTableWidgetItem(obj["name"].toString());
        ui->tableWidget->setItem(i, 0, nameItem);

        QTableWidgetItem *timeItem = new QTableWidgetItem("0:00");
        ui->tableWidget->setItem(i, 1, timeItem);

        QTableWidgetItem *statusItem = new QTableWidgetItem("Ready");
        ui->tableWidget->setItem(i, 2, statusItem);
    }
    ui->tableWidget->resizeColumnsToContents();
}

void swapRows(QTableWidget *table, int row1, int row2) {
    if (row1 >= table->rowCount() || row2 >= table->rowCount() || row1 < 0 ||
        row2 < 0) {
        qDebug() << "wrong row index: " << row1 << ", " << row2;
    }

    for (int i = 0; i < table->columnCount(); i++) {
        QTableWidgetItem *item1 = table->takeItem(row1, i);
        QTableWidgetItem *item2 = table->takeItem(row2, i);

        table->setItem(row1, i, item2);
        table->setItem(row2, i, item1);
    }
}

} // namespace

ExamTab::ExamTab(QWidget *parent) : QWidget(parent), ui(new Ui::studytab) {
    ui->setupUi(this);

    loadPatients();
    loadExams(ui);
}

ExamTab::~ExamTab() { delete ui; }

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

void ExamTab::on_pushButton_3_clicked() {
    // up button
    int curRow = ui->tableWidget->currentRow();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == 0) {
        qDebug() << "alreay the first exam";
        return;
    }

    swapRows(ui->tableWidget, curRow, curRow - 1);
}

void ExamTab::on_pushButton_4_clicked() {
    // down button
    int curRow = ui->tableWidget->currentRow();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == ui->tableWidget->rowCount() - 1) {
        qDebug() << "already the last exam";
        return;
    }

    swapRows(ui->tableWidget, curRow, curRow + 1);
}

void ExamTab::on_pushButton_5_clicked()
{
    // remove button
    int curRow = ui->tableWidget->currentRow();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    ui->tableWidget->removeRow(curRow);
}


void ExamTab::on_pushButton_6_clicked()
{
    // copy button
    int curRow = ui->tableWidget->currentRow();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);

    for(int col=0;col<ui->tableWidget->columnCount();col++){
        QTableWidgetItem* item = ui->tableWidget->item(curRow, col)->clone();
        ui->tableWidget->setItem(rowCount, col, item);
    }
}


void ExamTab::on_pushButton_2_clicked()
{
    // edit button
    int curRow = ui->tableWidget->currentRow();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    ExamInfoDialog dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

