#include "store.h"

#include <QDir>

#include "utils.h"

namespace{

QString examPath(const QString& pid, const QString& eid){
    return QString("./patients/%1/%2").arg(pid, eid);
}

const static QString kRequestFileName = "request.json";
ExamRequest loadExamRequest(const QString& pid, const QString& eid){
    auto fpath = QString("%1/%2").arg(examPath(pid, eid), kRequestFileName);
    auto obj = json_utils::readFromFile(fpath).object();
    return ExamRequest(obj);
}

void saveExamRequest(const QString& pid, const QString& eid, ExamRequest request){
    auto fpath = QString("%1/%2").arg(examPath(pid, eid), kRequestFileName);
    json_utils::saveToFile(fpath, request.data());
}


} // namespace

namespace store{

/// @todo
JsonPatient loadPatient(const QString& pid){
    return JsonPatient();
}

/// @todo
void savePatient(const JsonPatient& patient){

}

/// @todo
Exam loadExam(const QString& pid, const QString& eid){
    return Exam();
}

/// @todo
void saveExam(const Exam& exam){
    auto pid = exam.patient()->id();
    auto eid = exam.id();
    auto dpath = examPath(pid, eid);

    QDir dir(dpath);
    if(dir.exists()){
        LOG_WARNING(QString("扫描文件被覆盖, pid: %1, eid: %2").arg(pid, eid));
    }else{
        dir.mkpath(".");
    }

    saveExamRequest(pid, eid, exam.request());

    // 存储response

}

} // namespace store
