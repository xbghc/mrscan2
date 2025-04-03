#ifndef EXAMHISTORY_H
#define EXAMHISTORY_H


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
    explicit ExamHistory(QJsonObject request, const QByteArray& response);

    const QList<QImage> images() const;

    void setPatient(const int patientId);
    void setRequest(QJsonObject request);
    void setResponse(const QByteArray& response);

    QString requestPath();
    QString responsePath();
    bool save();
private:
    int m_patientId=-1;
    QJsonObject m_request;
    QByteArray m_response;

    QString dirPath();
};

#endif // EXAMHISTORY_H
