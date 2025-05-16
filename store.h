#ifndef STORE_H
#define STORE_H

#include "exam.h"
#include "patient.h"

/**
 * @namespace store
 * @brief
 * 统一处理扫描文件的读写，管理扫描文件的目录结构，同时为HistoryModel提供类似于数据库的访问方式
 *
 * 目录结构为:
 *   - patientId 每个病人对应一个文件夹
 *     - patientInfo.json 存储病人信息
 *     - examId 每一次扫描对应一个文件夹
 *       - request.json 扫描参数
 *       - response.mrd 扫描结果数据
 *       - info.json
其他信息，比如扫描的id，开始和结束事件等，同时文件会包含一份完整的病人信息
 *
 * @note 尽量不要包含具体的文件内容到数据结构的转换过程，这应该在类的静态函数中实现
 */
namespace store {

QVector<JsonPatient> loadAllPatients();

void addPatient(const JsonPatient &patient);

void deletePatient(const QString &pid);

/// 加载病人信息
JsonPatient loadPatient(const QString &pid);
/// 保存病人信息
void savePatient(const JsonPatient &patient);



/// 加载扫描记录
Exam loadExam(const QString &pid, const QString &eid);
/// 保存扫描记录
void saveExam(const Exam &exam);

} // namespace store

#endif // STORE_H
