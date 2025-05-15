#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "exam.h"

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QMutex>

// 配置管理器 - 单例模式
class ConfigManager : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例
    static ConfigManager* instance();
    static const QString kConfigDir;
    
    void load(const QString& cname);
    void save(const QString& cname);
    
    QJsonValue get(const QString& cname, const QString& key);
    void set(const QString& cname, const QString& key, QJsonValue value);

    // 创建新的ID
    int generateId();
    
signals:
    // 配置变更信号
    void configChanged(const QString& configName, const QString& path);
    
private:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    QString fpath(const QString& configName);

    QMap<QString, QJsonValue> m_configs; /// @todo 考虑value直接用QJsonObject
};

class ExamConfig{
public:
    static const QString kName;
    struct Keys{
        const static QString InitExams;
    };

    /// @todo 添加set
    static QList<Exam> initialExams();
private:
    ExamConfig() = delete;
    ExamConfig(const ExamConfig&)=delete;
    ExamConfig& operator=(const ExamConfig&)=delete;
    ~ExamConfig() = delete;
};

#endif // CONFIGMANAGER_H 
