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
    enum class Type{
        New,
        Edit
    };

    explicit PatientInfoDialog(QWidget *parent = nullptr);
    ~PatientInfoDialog();

    QString id() const;
    QString name() const;
    QDate birthday() const;
    IPatient::Gender gender() const;

    void setId(QString id);
    void setName(QString name);
    void setBithDay(QDate birthday);
    void setGender(IPatient::Gender gender);
    void setPatient(const IPatient* patient);

    void clear();
    Type type();
    void setType(Type type);

private:
    QString m_id;
    Type m_type;

    std::unique_ptr<Ui::PatientInfoDialog> ui;
};

#endif // PATIENTINFODIALOG_H
