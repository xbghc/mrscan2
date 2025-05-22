#include "store.h"

#include <QDir>

#include "mrdresponse.h"
#include "utils.h"

namespace {

const auto kRootDir = "./patients";

const auto kRequestFileName = "request.json";
const auto kResponseFileName = "response.mrd";
const auto kExamInfoFileName = "info.json";
const auto kPatientInfoFileName = "patient.json";


QString patientInfoPath(const QString &pid) {
    return QString("%1/%2").arg(store::pdir(pid), kPatientInfoFileName);
}

QString reqFilePath(const QString &pid, const QString &eid) {
    return QString("%1/%2").arg(store::edir(pid, eid), kRequestFileName);
}

QString respFilePath(const QString &pid, const QString &eid) {
    return QString("%1/%2").arg(store::edir(pid, eid), kResponseFileName);
}

QString examInfoFilePath(const QString &pid, const QString &eid) {
    return QString("%1/%2").arg(store::edir(pid, eid), kExamInfoFileName);
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

IExamResponse *loadResponse(const QString &pid, const QString &eid) {
    auto fpath = respFilePath(pid, eid);

    /// @note 这里将来需要判断文件内容决定返回哪一种实现
    return new MrdResponse(file_utils::read(fpath));
}

void saveResponse(const QString &pid, const QString &eid, IExamResponse *resp) {
    auto fpath = respFilePath(pid, eid);

    file_utils::save(fpath, resp->bytes());
}

void loadExamInfo(Exam &exam, const QString &pid, const QString &eid) {
    if (!(exam.id() == eid)) {
        exam.setId(eid);
    }

    auto fpath = examInfoFilePath(pid, eid);
    auto infoObj = json_utils::readFromFile(fpath).object();

    auto startTime = QDateTime::fromString(infoObj["startTime"].toString());
    exam.setStartTime(startTime);
    auto endTime = QDateTime::fromString(infoObj["endTime"].toString());
    exam.setEndTime(endTime);

    auto patient = store::loadPatient(pid);
    exam.setPatient(patient);
}

void saveExamInfo(const Exam &exam) {
    auto pid = exam.patient()->id();
    auto eid = exam.id();

    QJsonObject infoObj;

    /// @note 本来应该存储病人所有信息，但是这些后面反正要改
    /// save只保存id，在load中调用loadPatient，外部感知是一样的
    infoObj["patient"] = pid;

    infoObj["id"] = eid;
    infoObj["startTime"] = exam.startTime().toString();
    infoObj["endTime"] = exam.endTime().toString();

    auto fpath = examInfoFilePath(pid, eid);
    json_utils::saveToFile(fpath, infoObj);
}

} // namespace

namespace store {

QString pdir(const QString &pid) { return QString("%1/%2").arg(kRootDir, pid); }

QString edir(const QString &pid, const QString &eid) {
    return QString("%1/%2").arg(pdir(pid), eid);
}

IPatient* loadPatient(const QString &pid) {
    auto path = patientInfoPath(pid);
    auto jsonObj = json_utils::readFromFile(path).object();
    auto ptr = new JsonPatient(jsonObj);
    return ptr;
}

void savePatient(IPatient* patient) {
    auto path = patientInfoPath(patient->id());
    auto bytes = patient->bytes();
    file_utils::save(path, bytes);
}

Exam loadExam(const QString &pid, const QString &eid) {
    Exam exam;

    auto request = loadExamRequest(pid, eid);
    auto response = loadResponse(pid, eid);
    loadExamInfo(exam, pid, eid);

    exam.setRequest(request);
    exam.setResponse(response);

    return exam;
}

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

    saveResponse(pid, eid, exam.response());

    saveExamInfo(exam);
}

QStringList patientEntries(){
    QDir root(kRootDir);
    return root.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

QStringList examEntries(const QString& pid){
    QDir dir(pdir(pid));
    QStringList eids;
    for(const auto& fname:dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)){
        eids << fname;
    }
    return eids;
}

/**
 * @brief 加载所有病人
 * @detial 遍历根文件夹中的文件夹列表，每一个文件夹都代表一个病人
 */
QVector<IPatient*> loadAllPatients() {
    QVector<IPatient*> patients;

    for (const auto &pid : patientEntries()) {
        patients.push_back(loadPatient(pid));
    }

    return patients;
}

void deletePatient(const QString &pid) {
    auto dpath = pdir(pid);
    QDir dir(dpath);
    dir.removeRecursively();
}

void addPatient(IPatient* patient) {
    QDir dir(pdir(patient->id()));
    if (dir.exists()) {
        LOG_WARNING(QString("病人信息被意外的覆盖了, pid: %1").arg(patient->id()));
    }
    dir.mkpath(".");

    savePatient(patient);
}

IPatient* createNewPatient(const QString& id, const QString& name, const QDate& birthday, IPatient::Gender gender){
    auto patient = new JsonPatient();
    patient->setId(id);
    patient->setName(name);
    patient->setBirthday(birthday);
    patient->setGender(gender);
    return patient;
}

std::unordered_map<QString, QStringList> examMap(){
    std::unordered_map<QString, QStringList> map;

    for(const auto& pid:patientEntries()){
        map[pid] = examEntries(pid);
    }

    return map;
}

} // namespace store
