#include <QJsonDocument>

#include "examhistory.h"
#include "QImagesWidget/mrdparser.h"

namespace{
QDir getExamDir(int patientId, int examId){
    return QDir(QString("./patients/%1/%2").arg(patientId).arg(examId));
}
}

ExamHistory::ExamHistory() {}

ExamHistory::ExamHistory(int patientId, int examId)
{
    QDir dir = getExamDir(patientId, examId);
    if(!dir.exists()){
        qDebug() << "exam dir not exists";
        return;
    }

    QFile requestFile(dir.filePath("request.json"));
    if(!requestFile.open(QIODevice::ReadOnly)){
        qDebug() << "open " << requestFile.fileName() << " failed";
    }
    m_request = QJsonDocument::fromJson(requestFile.readAll()).object();

    QFile responseFile(dir.filePath("response.dat"));
    if(!responseFile.open(QIODevice::ReadOnly)){
        qDebug() << "open " << responseFile.fileName() << " failed";
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

bool ExamHistory::save()
{
    if(m_patientId == -1){
        qDebug() << "history model: patient id is not set";
        return false;
    }
    int requestId = m_request["id"].toInt();
    QDir dir = getExamDir(m_patientId, requestId);
    if(!dir.exists() && !dir.mkpath(".")){
        return false;
    }

    QFile requestFile(dir.filePath("request.json"));
    if(!requestFile.open(QIODevice::WriteOnly)){
        return false;
    }
    QJsonDocument doc(m_request);
    requestFile.write(doc.toJson(QJsonDocument::Indented));
    requestFile.close();

    QFile responseFile(dir.filePath("response.dat"));
    if(!responseFile.open(QIODevice::WriteOnly)){
        return false;
    }
    responseFile.write(m_response);

    return true;
}
