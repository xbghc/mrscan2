#include "patient.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QList<Patient> Patient::patientsList;
const QString Patient::kDateFormat = "yyyy-MM-dd";
const QString Patient::kDirPath = "./patients";

Patient::Patient() {}

Patient::Patient(int _id, QString _name, QDate _birthday, bool _isMale)
    : id(_id), name(_name), birthday(_birthday), isMale(_isMale) {}

int Patient::getId()
{
    return id;
}

bool Patient::setName(QString other) {
    name = other;
    return true;
}

QString Patient::getName() { return name; }

bool Patient::setBirthday(QDate other) {
    birthday = other;
    return true;
}

QDate Patient::getBirthday() { return birthday; }

bool Patient::setGender(bool other) {
    isMale = other;
    return true;
}

bool Patient::getGender() { return isMale; }

bool Patient::loadPatients() {
    const static QString kFilePath = "./patients/info.json";

    QDir dir(kDirPath);
    if (!dir.exists() && !dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << kDirPath;
        return false;
    }

    QFile file(kFilePath);
    if (!file.exists()) {
        savePatients();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "failed to open file: " << kFilePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonArray jsonArray = QJsonDocument::fromJson(data).array();
    QList<Patient> list;
    for (auto &&qjv : jsonArray) {
        QJsonObject obj = qjv.toObject();
        list.append(
            Patient(obj["id"].toInt(), obj["name"].toString(),
                    QDate::fromString(obj["birthday"].toString(), kDateFormat),
                    obj["isMale"].toBool()));
    }

    patientsList = list;
    return true;
}

bool Patient::savePatients() {
    QJsonArray jsonArray;
    for (auto &patient : patientsList) {
        jsonArray.push_back(
            QJsonObject{{"id", patient.getId()},
                        {"name", patient.getName()},
                        {"birthday", patient.getBirthday().toString(kDateFormat)},
                        {"isMale", patient.getGender()}});
    }

    const static QString kFilePath = "./patients/info.json";

    QDir dir(kDirPath);
    if (!dir.exists() && dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << kDirPath;
        return false;
    }

    QFile file(kFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "failed to open file: " << kFilePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument();
    doc.setArray(jsonArray);
    file.write(doc.toJson());
    return true;
}

int Patient::getNextId() {
    const static QString kFilePath = "./patients/nextId";
    QDir dir(kDirPath);
    if (!dir.exists() && dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << kDirPath;
        return -1;
    }

    QFile file(kFilePath);
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to open file: " << kFilePath;
            return -1;
        } else {
            file.write("1");
            return 0;
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "failed to open file: " << kFilePath;
        return -1;
    }
    return file.readAll().toInt();
}

bool Patient::setNextId(int id)
{
    const static QString kFilePath = "./patients/nextId";
    QFile file(kFilePath);

    if(getNextId() + 1 != id){
        qDebug() << "wrong id: " << id;
        return false;
    }

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "failed to write to: " << kFilePath;
        return false;
    }

    file.write(QByteArray::number(id));
    return true;
}

const Patient Patient::getPatient(int id)
{
    for(const auto& p:patientsList){
        if(p.id == id){
            return p;
        }
    }

    qDebug() << "id " << id << " doesn't exist";
    return Patient(-1, "", QDate(), false);
}

bool Patient::addPatient(QString _name, QDate _birthday, bool _isMale) {
    int nextId = getNextId();
    Patient _patient(nextId, _name, _birthday, _isMale);
    patientsList.push_back(_patient);

    return setNextId(nextId + 1) && savePatients();
}

bool Patient::replacePatientById(int id, QString _name, QDate _birthday, bool _isMale)
{
    for(auto& p:patientsList){
        if(p.id == id){
            p.setName(_name);
            p.setBirthday(_birthday);
            p.setGender(_isMale);
            savePatients();
            return true;
        }
    }
    return false;
}

bool Patient::removePatient(int id)
{
    for(int i = 0; i < patientsList.size(); i++){
        if(patientsList[i].id == id){
            patientsList.removeAt(i);
            savePatients();
            return true;
        }
    }
    return false;
}
