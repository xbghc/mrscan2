#include "examitem.h"

ExamItem::ExamItem(const QJsonObject& data)
    : m_data(data)
    , m_status(Status::Ready)
    , m_time("0:00")
{
}

QJsonObject ExamItem::parameters() const
{
    return m_data.contains("parameters") ? m_data["parameters"].toObject() : QJsonObject();
}

void ExamItem::setParameters(const QJsonObject& parameters)
{
    QJsonObject data = m_data;
    data["parameters"] = parameters;
    m_data = data;
}

QJsonObject ExamItem::response() const
{
    return m_data.contains("response") ? m_data["response"].toObject() : QJsonObject();
}

void ExamItem::setResponse(const QJsonObject& response)
{
    QJsonObject data = m_data;
    data["response"] = response;
    m_data = data;
}

QString ExamItem::name() const
{
    return m_data.contains("name") ? m_data["name"].toString() : "未命名";
}

int ExamItem::id() const
{
    return m_data.contains("id") ? m_data["id"].toInt() : -1;
}

void ExamItem::setId(int id)
{
    QJsonObject data = m_data;
    data["id"] = id;
    m_data = data;
}

QString ExamItem::statusText(Status status)
{
    switch (status) {
    case Status::Ready:
        return "Ready";
    case Status::Processing:
        return "Processing";
    case Status::Done:
        return "Done";
    default:
        return "Unknown";
    }
} 