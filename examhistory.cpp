#include <QDir>
#include <QJsonDocument>

#include "examhistory.h"

namespace{
}

ExamHistory::ExamHistory() {}

ExamHistory::ExamHistory(QJsonObject request, const QByteArray& response)
    : m_request(request)
{
m_response = ScannerResponse::fromBytes(response);
}

const QList<QImage> ExamHistory::images() const
{
    return m_response.getImages();
}

void ExamHistory::setPatient(const int patientId)
{
    m_patientId = patientId;
}

void ExamHistory::setRequest(QJsonObject request)
{
    m_request = request;
}

void ExamHistory::setResponse(ScannerResponse response)
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
    QString dirpath = QString("./patients/%1/%2").arg(m_patientId).arg(requestId);
    QDir dir(dirpath);
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
    responseFile.write(m_response.getRawData());

    return true;
}
