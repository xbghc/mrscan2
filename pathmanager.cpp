#include "pathmanager.h"
#include "utils.h"

const QString PathManager::ROOT_DIR = "./";
const QString PathManager::PATIENTS_DIR = "patients";

QString PathManager::getPatientDir(int patientId)
{
    return QDir(ROOT_DIR).filePath(QString("%1/%2").arg(PATIENTS_DIR).arg(patientId));
}

QString PathManager::getExamDir(int patientId, int examId)
{
    return QDir(getPatientDir(patientId)).filePath(QString("%1").arg(examId));
}

QString PathManager::getRequestFilePath(int patientId, int examId)
{
    return QDir(getExamDir(patientId, examId)).filePath("request.json");
}

QString PathManager::getResponseFilePath(int patientId, int examId, int channelId)
{
    return QDir(getExamDir(patientId, examId)).filePath(QString("response#%1.dat").arg(channelId));
}

bool PathManager::ensureDirExists(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        bool success = dir.mkpath(".");
        if (!success) {
            LOG_ERROR(QString("Failed to create directory: %1").arg(path));
        }
        return success;
    }
    return true;
}

bool PathManager::ensurePatientDirExists(int patientId)
{
    QString path = getPatientDir(patientId);
    return ensureDirExists(path);
}

bool PathManager::ensureExamDirExists(int patientId, int examId)
{
    QString path = getExamDir(patientId, examId);
    return ensureDirExists(path);
}

bool PathManager::patientDirExists(int patientId)
{
    QDir dir(getPatientDir(patientId));
    return dir.exists();
}

bool PathManager::examDirExists(int patientId, int examId)
{
    QDir dir(getExamDir(patientId, examId));
    return dir.exists();
}

QStringList PathManager::getAllPatientIds()
{
    QDir rootDir(QDir(ROOT_DIR).filePath(PATIENTS_DIR));
    rootDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    return rootDir.entryList();
}

QStringList PathManager::getExamIdsForPatient(int patientId)
{
    QDir patientDir(getPatientDir(patientId));
    patientDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    return patientDir.entryList();
} 