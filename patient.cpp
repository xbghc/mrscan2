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

namespace {} // namespace

JsonPatient::JsonPatient(QJsonObject data)
    :m_data(data)
{

}

IPatient *JsonPatient::clone() const { return new JsonPatient(m_data); }

QString JsonPatient::id() const {
    auto v = json_utils::get(m_data, Keys::Id, -1);
    return QString::number(v);
}

QString JsonPatient::name() const {
    return json_utils::get(m_data, Keys::Name, "unnamed");
}

JsonPatient::Gender JsonPatient::gender() const {
    auto g =
        json_utils::get(m_data, Keys::Gender, static_cast<int>(Gender::Male));
    return static_cast<Gender>(g);
}

QDate JsonPatient::birthday() const {
    auto dateStr = json_utils::get(m_data, Keys::Birthday, "");
    if (dateStr.isEmpty()) {
        return QDate::currentDate();
    }
    return QDate::fromString(dateStr, kDateFormat);
}

void JsonPatient::setId(const QString &other) {
    auto old = id();
    if (old.toInt() >= 0 && old != other) {
        LOG_WARNING("A valid ID is being overwritten");
    }
    m_data[Keys::Id] = other.toInt();
}

void JsonPatient::setName(const QString &other) { m_data[Keys::Name] = other; }

void JsonPatient::setGender(Gender other) {
    m_data[Keys::Gender] = static_cast<int>(other);
}

void JsonPatient::setBirthday(int year, int month, int day) {
    auto date = QDate(year, month, day);
    setBirthday(date);
}

void JsonPatient::setBirthday(QDate other) {
    m_data[Keys::Birthday] = other.toString(kDateFormat);
}

QByteArray JsonPatient::bytes() const
{
    QJsonDocument doc(m_data);
    return doc.toJson();
}
