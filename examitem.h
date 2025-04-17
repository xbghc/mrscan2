#ifndef EXAMITEM_H
#define EXAMITEM_H

#include <QJsonObject>
#include <QString>

// 将单个检查的数据封装到一个类中
class ExamItem {
public:
    // 检查状态枚举
    enum class Status {
        Ready = 0,      // 准备就绪
        Processing = 1, // 扫描中
        Done = 2,       // 已完成
    };
    
    ExamItem(const QJsonObject& data = QJsonObject());
    
    // 获取检查数据
    QJsonObject data() const { return m_data; }
    void setData(const QJsonObject& data) { m_data = data; }
    
    // 获取与设置参数
    QJsonObject parameters() const;
    void setParameters(const QJsonObject& parameters);
    
    // 获取与设置响应数据
    QJsonObject response() const;
    void setResponse(const QJsonObject& response);
    
    // 获取检查名称
    QString name() const;
    
    // 状态管理
    Status status() const { return m_status; }
    void setStatus(Status status) { m_status = status; }
    
    // 获取与设置计时
    QString time() const { return m_time; }
    void setTime(const QString& time) { m_time = time; }
    
    // ID管理
    int id() const;
    void setId(int id);
    
    // 状态文本
    static QString statusText(Status status);
    
private:
    QJsonObject m_data;       // 包含所有检查数据的JSON对象
    Status m_status;          // 当前状态
    QString m_time;           // 计时字符串
};

#endif // EXAMITEM_H 