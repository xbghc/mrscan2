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

QString PatientInfoDialog::id() const
{
    return m_id;
}

QString PatientInfoDialog::name() const
{
    return ui->nameEdit->text();
}

QDate PatientInfoDialog::birthday() const
{
    return ui->birthdayEdit->date();
}

IPatient::Gender PatientInfoDialog::gender() const
{
    if(ui->isMaleRadioButton->isChecked()){
        return IPatient::Gender::Male;
    }
    return IPatient::Gender::Female;
}

void PatientInfoDialog::setId(QString id)
{
    m_id = id;
}

void PatientInfoDialog::setName(QString name)
{
    ui->nameEdit->setText(name);
}

void PatientInfoDialog::setBithDay(QDate birthday)
{
    ui->birthdayEdit->setDate(birthday);
}

void PatientInfoDialog::setGender(IPatient::Gender gender)
{
    bool isMale;
    if(gender == IPatient::Gender::Male){
        isMale = true;
    }else{
        isMale = false;
    }

    ui->isMaleRadioButton->setChecked(isMale);
    ui->isFemaleRadioButton->setChecked(!isMale);
}

void PatientInfoDialog::clear()
{
    setId("");
    setName("");
    setBithDay(QDate::currentDate());
    setGender(IPatient::Gender::Male);
}

PatientInfoDialog::Type PatientInfoDialog::type()
{
    return m_type;
}

void PatientInfoDialog::setType(Type type)
{
    m_type = type;
}

