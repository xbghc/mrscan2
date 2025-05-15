#ifndef STORE_H
#define STORE_H

#include "patient.h"
#include "exam.h"

namespace store{

/// 加载病人信息
JsonPatient loadPatient(const QString& pid);
/// 保存病人信息
void savePatient(const JsonPatient& patient);

/// 加载扫描记录
Exam loadExam(const QString& pid, const QString& eid);
/// 保存扫描记录
void saveExam(const Exam& exam);

}

#endif // STORE_H
