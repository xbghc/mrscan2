#ifndef EXAMHISTORY_H
#define EXAMHISTORY_H

#include "scannerresponse.h"

#include <QByteArray>
#include <QDir>
#include <QImage>
#include <QJsonObject>
#include <QList>

class ExamHistory
{
public:
    explicit ExamHistory();
    explicit ExamHistory(int patientId, int examId);
    explicit ExamHistory(QJsonObject request, const QByteArray& responseData);
    explicit ExamHistory(QJsonObject request, ScannerResponse response);

    const QList<QImage> images() const;

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
