#ifndef STORE_H
#define STORE_H

#include "exam.h"
#include "patient.h"

/**
 * @namespace store
 * @brief
 * Unified handling of scan file read/write, managing scan file directory structure, and providing database-like access for HistoryModel
 *
 * Directory structure:
 *   - patientId Each patient corresponds to a folder
 *     - patientInfo.json Store patient information
 *     - examId Each scan corresponds to a folder
 *       - request.json Scan parameters
 *       - response.mrd Scan result data
 *       - info.json
Other information, such as scan ID, start and end events, etc., and the file will contain a complete copy of patient information
 *
 * @note Try not to include specific file content to data structure conversion process, this should be implemented in static functions of the class
 */
namespace store {

/// Patient directory
QString pdir(const QString &pid);

/// Scan directory
QString edir(const QString &pid, const QString &eid);

QVector<IPatient*> loadAllPatients();

void addPatient(IPatient* patient);
IPatient* createNewPatient(const QString& id, const QString& name, const QDate& birthday, IPatient::Gender gender);

void deletePatient(const QString &pid);

/// Load patient information
IPatient* loadPatient(const QString &pid);
/// Save patient information
void savePatient(IPatient* patient);

/// Load scan record
Exam loadExam(const QString &pid, const QString &eid);
/// Save scan record
void saveExam(const Exam &exam);

/// Return patient ID list
QStringList patientEntries();

/// Get pid: QVector<eid> dictionary
std::unordered_map<QString, QStringList> examMap();

/// Return patient's eid list
QStringList examEntries(const QString& pid);

} // namespace store

#endif // STORE_H
