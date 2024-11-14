#ifndef PATIENT_H
#define PATIENT_H

#include <QDate>
#include <QList>

class Patient
{
public:
    Patient();
    Patient(int _id, QString _name, QDate _birthday, bool _isMale);

    int getId();
    bool setName(QString other);
    QString getName();
    bool setBirthday(QDate other);
    QDate getBirthday();
    bool setGender(bool other);
    bool getGender();

    const static QString kDateFormat;
    const static QString kDirPath;
    static bool loadPatients();
    static bool savePatients();
    static QList<Patient> patientsList;
    static int getNextId();
    static bool setNextId(int id);
    static const Patient getPatient(int id);
    static bool addPatient(QString _name, QDate _birthday, bool _isMale);
    static bool replacePatientById(int id, QString _name, QDate _birthday, bool _isMale);
    static bool removePatient(int id);
private:
    int id;
    QString name;
    QDate birthday;
    bool isMale;
};

#endif // PATIENT_H
