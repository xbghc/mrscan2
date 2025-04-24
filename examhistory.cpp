#include <QJsonDocument>

#include "examhistory.h"
#include "mrdparser.h"
#include "pathmanager.h"
#include "utils.h"

ExamHistory::ExamHistory() {}

ExamHistory::ExamHistory(int patientId, int examId)
{
    if (!PathManager::examDirExists(patientId, examId)) {
        LOG_WARNING(QString("Exam directory does not exist, patientId: %1, examId: %2").arg(patientId).arg(examId));
        return;
    }

    QString requestFilePath = PathManager::getRequestFilePath(patientId, examId);
    QFile requestFile(requestFilePath);
    if (!requestFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("Failed to open %1").arg(requestFile.fileName()));
    }
    m_request = QJsonDocument::fromJson(requestFile.readAll()).object();

    QString responseFilePath = PathManager::getResponseFilePath(patientId, examId);
    QFile responseFile(responseFilePath);
    if (!responseFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("Failed to open %1").arg(responseFile.fileName()));
    }

    m_response = responseFile.readAll();
}

ExamHistory::ExamHistory(QJsonObject request, const QByteArray& response)
    : m_request(request)
{
    m_response = response;
}

const QList<QImage> ExamHistory::images() const
{
    auto mrd = MrdParser::parse(m_response);
    return MrdParser::reconImages(mrd);
}

void ExamHistory::setPatient(const int patientId)
{
    m_patientId = patientId;
}

void ExamHistory::setRequest(QJsonObject request)
{
    m_request = request;
}

void ExamHistory::setResponse(const QByteArray& response)
{
    m_response = response;
}

QString ExamHistory::requestPath()
{
    if (m_patientId == -1) {
        LOG_ERROR("Patient ID is not set");
        return "";
    }
    
    int requestId = m_request["id"].toInt();
    return PathManager::getRequestFilePath(m_patientId, requestId);
}

QString ExamHistory::responsePath()
{
    if (m_patientId == -1) {
        LOG_ERROR("Patient ID is not set");
        return "";
    }
    
    int requestId = m_request["id"].toInt();
    return PathManager::getResponseFilePath(m_patientId, requestId);
}

bool ExamHistory::save()
{
    if (m_patientId == -1) {
        LOG_ERROR("Patient ID is not set");
        return false;
    }
    
    int requestId = m_request["id"].toInt();
    
    if (!PathManager::ensureExamDirExists(m_patientId, requestId)) {
        LOG_ERROR("Failed to create exam directory");
        return false;
    }
    
    QFile requestFile(requestPath());
    if (!requestFile.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Failed to open file for writing: %1").arg(requestFile.fileName()));
        return false;
    }
    QJsonDocument doc(m_request);
    requestFile.write(doc.toJson(QJsonDocument::Indented));
    requestFile.close();

    QFile responseFile(responsePath());
    if (!responseFile.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Failed to open file for writing: %1").arg(responseFile.fileName()));
        return false;
    }
    responseFile.write(m_response);
    responseFile.close();

    return true;
}

QString ExamHistory::dirPath()
{
    if (m_patientId == -1) {
        LOG_ERROR("Patient ID is not set");
        return "";
    }

    int requestId = m_request["id"].toInt();
    
    if (!PathManager::ensureExamDirExists(m_patientId, requestId)) {
        LOG_ERROR("Failed to create exam directory");
        return "";
    }
    
    return PathManager::getExamDir(m_patientId, requestId);
}
