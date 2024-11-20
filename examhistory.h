#ifndef EXAMHISTORY_H
#define EXAMHISTORY_H

#include "scannerresponse.h"

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>

class ExamHistory
{
public:
    ExamHistory();
    ExamHistory(QJsonObject request, const QByteArray& responseData);
    ExamHistory(QJsonObject request, ScannerResponse response);

    QList<QImage> images();

    void setPatient(const int patientId);
    void setRequest(QJsonObject request);
    void setResponse(ScannerResponse response);

    bool save();
private:
    int m_patientId=-1;
    QJsonObject m_request;
    ScannerResponse m_response;
};

#endif // EXAMHISTORY_H
