#include "patientinfodialog.h"
#include "ui_patientinfodialog.h"

PatientInfoDialog::PatientInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PatientInfoDialog)
{
    ui->setupUi(this);
}

PatientInfoDialog::~PatientInfoDialog()
{
}

void PatientInfoDialog::setPatient(Patient *patient)
{
    if(patient==nullptr){
        id = -1;
    }else{
        id = patient->getId();
        ui->nameEdit->setText(patient->getName());
        bool isMale = patient->getGender();
        ui->isMaleRadioButton->setChecked(isMale);
        ui->isFemaleRadioButton->setChecked(!isMale);
        ui->birthdayEdit->setDate(patient->getBirthday());
    }
}

void PatientInfoDialog::on_buttonBox_accepted()
{
    if(id < 0){
        Patient::addPatient(ui->nameEdit->text(), ui->birthdayEdit->date(), ui->isMaleRadioButton->isChecked());
    }else{
        if(!Patient::replacePatientById(id, ui->nameEdit->text(), ui->birthdayEdit->date(), ui->isMaleRadioButton->isChecked()))
        {
            qDebug() << "failed to edit info, patient id: " << id;
            return;
        }
    }
    accept();
}

