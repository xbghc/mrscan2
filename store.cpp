#include "store.h"

#include <QDir>

#include "utils.h"

namespace {

const auto kRootDir = "./patients";

const auto kRequestFileName = "request.json";
const auto kResponseFileName = "response.mrd";
const auto kExamInfoFileName = "info.json";
const auto kPatientInfoFileName = "patient.json";

/// 病人目录
QString pdir(const QString &pid){
    return QString("%1/%2").arg(kRootDir, pid);
}

QString patientInfoPath(const QString &pid) {
    return QString("%1/%2").arg(pdir(pid), kPatientInfoFileName);
}

/// 扫描目录
QString edir(const QString &pid, const QString &eid) {
    return QString("%1/%2").arg(pdir(pid), eid);
}

QString reqFilePath(const QString& pid, const QString& eid){
    return QString("%1/%2").arg(edir(pid, eid), kRequestFileName);
}

QString respFilePath(const QString& pid, const QString& eid){
    return QString("%1/%2").arg(edir(pid, eid), kResponseFileName);
}

QString examInfoFilePath(const QString& pid, const QString& eid){
    return QString("%1/%2").arg(edir(pid, eid), kExamInfoFileName);
}

ExamRequest loadExamRequest(const QString &pid, const QString &eid) {
    auto fpath = reqFilePath(pid, eid);
    auto obj = json_utils::readFromFile(fpath).object();
    return ExamRequest(obj);
}

void saveExamRequest(const QString &pid, const QString &eid,
                     ExamRequest request) {
    auto fpath = reqFilePath(pid, eid);
    json_utils::saveToFile(fpath, request.data());
}

} // namespace

namespace store {

JsonPatient loadPatient(const QString &pid) {
    auto path = patientInfoPath(pid);
    auto jsonObj = json_utils::readFromFile(path).object();
    return JsonPatient(jsonObj);
}

void savePatient(const JsonPatient &patient) {
    auto path = patientInfoPath(patient.id());
    auto jsonObj = patient.json();
    json_utils::saveToFile(path, jsonObj);
}

/// @todo
Exam loadExam(const QString &pid, const QString &eid) { return Exam(); }

/// @todo
void saveExam(const Exam &exam) {
    auto pid = exam.patient()->id();
    auto eid = exam.id();
    auto dpath = edir(pid, eid);

    QDir dir(dpath);
    if (dir.exists()) {
        LOG_WARNING(QString("扫描文件被覆盖, pid: %1, eid: %2").arg(pid, eid));
    } else {
        dir.mkpath(".");
    }

    saveExamRequest(pid, eid, exam.request());

    // 存储response
}

/**
 * @brief 加载所有病人
 * @detial 遍历根文件夹中的文件夹列表，每一个文件夹都代表一个病人
 */
QVector<JsonPatient> loadAllPatients()
{
    QVector<JsonPatient> patients;

    QDir root(kRootDir);
    for(const auto& pid:root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)){
        patients.push_back(loadPatient(pid));
    }

    return patients;
}

void removePatient(const QString &pid)
{
    /// @todo
}

void addPatient(const JsonPatient &patient)
{
    QDir dir(pdir(patient.id()));
    if(dir.exists()){
        LOG_WARNING(QString("病人信息被意外的覆盖了, pid: %1").arg(patient.id()));
    }
    dir.mkpath(".");

    savePatient(patient);
}

} // namespace store
