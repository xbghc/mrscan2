#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <QString>
#include <QDir>

class PathManager
{
public:
    static const QString ROOT_DIR;
    static const QString PATIENTS_DIR;
    
    static QString getPatientDir(int patientId);
    static QString getExamDir(int patientId, int examId);
    static QString getRequestFilePath(int patientId, int examId);
    static QString getResponseFilePath(int patientId, int examId, int channelId = 1);
    
    static bool ensureDirExists(const QString& path);
    static bool ensurePatientDirExists(int patientId);
    static bool ensureExamDirExists(int patientId, int examId);
    
    static QStringList getAllPatientIds();
    static QStringList getExamIdsForPatient(int patientId);
    
    static bool examDirExists(int patientId, int examId);
    static bool patientDirExists(int patientId);

    static void setHistoryPathFormat(const QString& format);
    static QString getHistoryPathFormat();

private:
    static QString m_historyPathFormat;
};

#endif // PATHMANAGER_H 