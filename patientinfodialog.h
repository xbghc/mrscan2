#ifndef PATIENTINFODIALOG_H
#define PATIENTINFODIALOG_H

#include <QDialog>
#include <memory>

#include "patient.h"

namespace Ui {
class PatientInfoDialog;
}

class PatientInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatientInfoDialog(QWidget *parent = nullptr);
    ~PatientInfoDialog();
    void setPatient(Patient* patient);

private slots:
    void on_buttonBox_accepted();

private:
    int id;
    std::unique_ptr<Ui::PatientInfoDialog> ui;
};

#endif // PATIENTINFODIALOG_H
