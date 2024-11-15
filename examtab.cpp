#include "examtab.h"
#include "examinfodialog.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "ui_examtab.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

namespace {} // namespace

ExamTab::ExamTab(QWidget *parent) : QWidget(parent), ui(new Ui::studytab) {
    ui->setupUi(this);

    loadPatients();

    exams = new ExamTableModel;
    ui->tableView->setModel(exams);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

ExamTab::~ExamTab() {
    delete ui;
    delete exams;
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

void ExamTab::on_pushButton_3_clicked() {
    // up button
    int curRow = ui->tableView->currentIndex().row();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == 0) {
        qDebug() << "alreay the first exam";
        return;
    }

    exams->swapRows(curRow, curRow - 1);
}

void ExamTab::on_pushButton_4_clicked() {
    // down button
    int curRow = ui->tableView->currentIndex().row();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    if (curRow == exams->rowCount() - 1) {
        qDebug() << "already the last exam";
        return;
    }

    exams->swapRows(curRow, curRow + 1);
}

void ExamTab::on_pushButton_5_clicked() {
    // remove button
    int curRow = ui->tableView->currentIndex().row();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    exams->removeRow(curRow);
}

void ExamTab::on_pushButton_6_clicked() {
    // copy button
    int curRow = ui->tableView->currentIndex().row();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    exams->copyRow(curRow);
}

void ExamTab::on_pushButton_2_clicked() {
    // edit button
    int curRow = ui->tableView->currentIndex().row();
    if (curRow == -1) {
        qDebug() << "no exam is selected";
        return;
    }

    ExamInfoDialog dialog(this);
    QJsonObject data = exams->getExamData(curRow);
    dialog.setData(data);
    connect(&dialog, &QDialog::accepted, this, [&]() {
        QJsonObject parameters = dialog.getParameters();
        this->exams->setExamParams(curRow, parameters);
    });

    dialog.setModal(true);
    dialog.exec();
}
