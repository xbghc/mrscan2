#include "patient.h"
#include "utils.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

const QString JsonPatient::Keys::Birthday = "birthday";
const QString JsonPatient::Keys::Gender = "gender";
const QString JsonPatient::Keys::Id = "id";
const QString JsonPatient::Keys::Name = "name";

namespace{

} // namespace

// TODO 路径从config中获取
const QString IPatient::kDirPath = "./patients";

JsonPatient::JsonPatient()
    :m_data(QJsonObject())
{

}

JsonPatient::JsonPatient(QJsonObject data)
    :m_data(data)
{

}

JsonPatient::JsonPatient(QJsonObject &&data)
    :m_data(data)
{

}

IPatient *JsonPatient::clone() const
{
    return new JsonPatient(m_data);
}

int JsonPatient::nextId()
{
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

QVector<JsonPatient> JsonPatient::loadPatients() {
    const static QString kFilePath = "./patients/info.json";

    QDir dir(kDirPath);
    if (!dir.exists() && !dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << kDirPath;
        return {};
    }

    QFile file(kFilePath);
    if (!file.exists()) {
        savePatients({});
        return {};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "failed to open file: " << kFilePath;
        return {};
    }

    QByteArray data = file.readAll();
    QJsonArray jsonArray = QJsonDocument::fromJson(data).array();
    QVector<JsonPatient> list;
    for (auto &&qjv : jsonArray) {
        auto obj = qjv.toObject();
        auto p = JsonPatient(obj);
        list.append(p);
    }
    return list;
}

void JsonPatient::savePatients(QVector<JsonPatient> patients)
{
    QJsonArray array;
    for (auto &patient : patients) {
        array.push_back(patient.json());
    }

    /// @todo 路径从config中读取
    const static QString kFilePath = "./patients/info.json";

    QDir dir(kDirPath);
    if (!dir.exists() && dir.mkpath(".")) {
        LOG_ERROR(QString("failed to mkdir: %1").arg(kDirPath));
        return;
    }

    QFile file(kFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("failed to open file: %1").arg(kFilePath));
    }

    QJsonDocument doc = QJsonDocument();
    doc.setArray(array);
    file.write(doc.toJson());
}


QString JsonPatient::id()
{
    auto v = json_utils::get(m_data, Keys::Id, -1);
    return QString::number(v);
}

QString JsonPatient::name()
{
    return json_utils::get(m_data, Keys::Name, "unnamed");
}

JsonPatient::Gender JsonPatient::gender()
{
    auto g = json_utils::get(m_data, Keys::Gender, static_cast<int>(Gender::Male));
    return static_cast<Gender>(g);
}

QDate JsonPatient::birthday()
{
    auto dateStr = json_utils::get(m_data, Keys::Birthday, "");
    if(dateStr.isEmpty()){
        return QDate::currentDate();
    }
    return QDate::fromString(dateStr, kDateFormat);
}

QJsonObject JsonPatient::json()
{
    return m_data;
}

void JsonPatient::setId(const QString& other)
{
    auto old = id();
    if(old.toInt() >= 0 && old != other){
        LOG_WARNING("一个有效id被覆盖");
    }
    m_data[Keys::Id] = other.toInt();
}

void JsonPatient::setName(const QString& other)
{
    m_data[Keys::Name] = other;
}

void JsonPatient::setGender(Gender other)
{
    m_data[Keys::Gender] = static_cast<int>(other);
}

void JsonPatient::setBirthday(int year, int month, int day)
{
    auto date = QDate(year, month, day);
    setBirthday(date);
}

void JsonPatient::setBirthday(QDate other)
{
    m_data[Keys::Birthday] = other.toString(kDateFormat);
}

