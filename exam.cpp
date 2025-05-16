#include "exam.h"

#include "utils.h"

const QString ExamRequest::Keys::Name = "name";
const QString ExamRequest::Keys::Params = "parameters";

Exam::Exam()
    :m_request(QJsonObject()),
    m_response(nullptr),
    m_patient(nullptr),
    m_status(Status::Ready)
{}

Exam::Exam(const Exam &other)
    : m_request(other.m_request),
    m_response(other.m_response?other.m_response->clone():nullptr),
    m_patient(other.m_patient?other.m_patient->clone():nullptr),
    m_status(other.m_status),
    m_startTime(other.m_startTime),
    m_endTime(other.m_endTime),
    m_id(other.m_id)
{
}

Exam::Exam(Exam &&other) noexcept
    :m_request(other.m_request),
    m_response(std::move(other.m_response)),
    m_patient(std::move(other.m_patient)),
    m_status(other.m_status),
    m_startTime(other.m_startTime),
    m_endTime(other.m_endTime),
    m_id(other.m_id)
{

}

Exam& Exam::operator=(const Exam& other){
    if(this==&other){
        return *this;
    }

    m_request = other.m_request;

    if(other.m_response){
        m_response.reset(other.m_response->clone());
    }else{
        m_response.reset();
    }

    if (other.m_patient) {
        m_patient.reset(other.m_patient->clone());
    } else {
        m_patient.reset();
    }

    m_status = other.m_status;
    m_startTime = other.m_startTime;
    m_endTime = other.m_endTime;
    m_id = other.m_id;

    return *this;
}

Exam& Exam::operator=(Exam&& other) noexcept{
    if(this==&other){
        return *this;
    }

    m_request = other.m_request;

    if(other.m_response){
        m_response = std::move(other.m_response);
    }

    if(other.m_patient){
        m_patient = std::move(other.m_patient);
    }

    m_status = other.m_status;
    m_startTime = other.m_startTime;
    m_endTime = other.m_endTime;
    m_id = other.m_id;

    return *this;
}

ExamRequest Exam::request() const
{
    return m_request;
}

void Exam::setRequest(const ExamRequest& other)
{
    m_request = other;
}

IExamResponse *Exam::response() const
{
    return m_response.get();
}

void Exam::setResponse(IExamResponse *other)
{
    m_response.reset(other);
}

IPatient *Exam::patient() const
{
    return m_patient.get();
}

void Exam::setPatient(IPatient *other)
{
    m_patient.reset(other);
}

int Exam::time() const
{
    return m_startTime.secsTo(m_endTime);
}

QDateTime Exam::startTime() const
{
    return m_startTime;
}

QDateTime Exam::endTime() const
{
    return m_endTime;
}

void Exam::setStartTime(QDateTime other)
{
    m_startTime = other;
}

void Exam::setEndTime(QDateTime other)
{
    m_endTime = other;
}

QString Exam::id() const
{
    return m_id;
}

void Exam::setId(const QString& other)
{
    m_id = other;
}

Exam::Status Exam::status() const
{
    return m_status;
}

void Exam::setStatus(Status other)
{
    m_status = other;
}

QVector<QVector<QImage> > Exam::images() const
{
    return m_response->images();
}

ExamRequest::ExamRequest(QJsonObject data)
    :m_data(data)
{

}

QString ExamRequest::name() const
{
    return json_utils::get(m_data, Keys::Name, "unnamed");
}

void ExamRequest::setName(QString other)
{
    m_data[Keys::Name] = other;
}

QJsonObject ExamRequest::params() const
{
    if(!m_data.contains(Keys::Params)){
        return QJsonObject();
    }

    return m_data[Keys::Params].toObject();
}

void ExamRequest::setParams(QJsonObject other, bool remainOld)
{
    auto obj = remainOld?params():QJsonObject();

    for(const auto& key:other.keys()){
        obj[key] = other[key];
    }

    m_data[Keys::Params] = obj;
}

QJsonObject ExamRequest::data() const
{
    return m_data;
}


