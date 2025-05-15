#ifndef EXAM_H
#define EXAM_H

#include "examresponse.h"
#include "patient.h"

#include <QJsonObject>

class ExamRequest{
public:
    struct Keys{
        const static QString Name;
        const static QString Params;
    };

    ExamRequest() = default;
    ExamRequest(QJsonObject data);

    QString name() const;
    void setName(QString other);

    QJsonObject params()const;
    void setParams(QJsonObject other, bool remainOld=true);

    QJsonObject data() const;
private:
    QJsonObject m_data;
};

class Exam
{
public:
    enum class Status{
        Ready = 0,
        Processing,
        Done
    };
    Exam();
    ~Exam()=default;
    Exam(const Exam& other); /// @todo 完善Exam, IPatient, IResponse的Rule of Five
    Exam& operator=(const Exam& other);
    Exam(Exam&& other)noexcept;
    Exam& operator=(Exam&& other)noexcept;

    ExamRequest request() const;
    void setRequest(const ExamRequest& other);

    IExamResponse* response() const;
    void setResponse(IExamResponse* other);

    IPatient* patient() const;
    void setPatient(IPatient* other);

    int time()const;
    void setStartTime(QDateTime other=QDateTime::currentDateTime());
    void setEndTime(QDateTime other=QDateTime::currentDateTime());

    QString id()const;
    void setId(const QString& other);

    Status status()const;
    void setStatus(Status other);

    QVector<QVector<QImage>> images()const;
private:
    ExamRequest m_request;
    std::unique_ptr<IExamResponse> m_response;
    std::shared_ptr<IPatient> m_patient;

    QDateTime m_startTime;
    QDateTime m_endTime;
    QString m_id;
    Status m_status;
};

#endif // EXAM_H
