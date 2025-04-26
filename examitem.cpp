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

QJsonObject ExamItem::toJsonObject() const
{
    QJsonObject json = m_data;  // 复制内部存储的数据
    
    // 确保状态和时间被正确保存
    json["status"] = static_cast<int>(m_status);
    json["time"] = m_time;
    
    return json;
}

bool ExamItem::fromJsonObject(const QJsonObject& json)
{
    if (json.isEmpty()) {
        return false;
    }
    
    // 保存JSON数据
    m_data = json;
    
    // 提取状态和时间
    if (json.contains("status")) {
        m_status = static_cast<Status>(json["status"].toInt());
    } else {
        m_status = Status::Ready;
    }
    
    if (json.contains("time")) {
        m_time = json["time"].toString();
    } else {
        m_time = "0:00";
    }
    
    return true;
} 