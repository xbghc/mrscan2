#include "examtab.h"
#include "patient.h"
#include "patientinfodialog.h"
#include "ui_examtab.h"

#include <QMessageBox>

ExamTab::ExamTab(QWidget *parent) : QWidget(parent), ui(new Ui::studytab) {
    ui->setupUi(this);

    loadPatients();
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
