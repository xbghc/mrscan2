#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <memory>

// 配置管理器 - 单例模式
class ConfigManager : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例
    static ConfigManager* instance();
    
    // 从文件加载配置
    bool loadConfig(const QString& configName);
    
    // 保存配置到文件
    bool saveConfig(const QString& configName);
    
    // 保存所有修改过的配置
    bool saveAllModified();
    
    // 获取配置值
    QJsonValue getValue(const QString& configName, const QString& path);
    QJsonObject getObject(const QString& configName, const QString& path = QString());
    QJsonArray getArray(const QString& configName, const QString& path = QString());
    
    // 设置配置值
    bool setValue(const QString& configName, const QString& path, const QJsonValue& value);
    bool setObject(const QString& configName, const QString& path, const QJsonObject& obj);
    bool setArray(const QString& configName, const QString& path, const QJsonArray& array);
    
    // 创建新的ID
    int generateId();
    
signals:
    // 配置变更信号
    void configChanged(const QString& configName, const QString& path);
    
private:
    // 私有构造函数和析构函数
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager();
    
    // 防止复制
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // 获取配置文件路径
    QString getConfigFilePath(const QString& configName);
    
    // 解析路径 (例如 "root.child1.child2")
    QStringList parsePath(const QString& path);
    
    // 查找指定路径的JSON值
    QJsonValue findValue(const QJsonValue& root, const QStringList& pathParts);
    
    // 设置指定路径的JSON值
    bool setValueAtPath(QJsonValue& root, const QStringList& pathParts, const QJsonValue& value);
    
    // 配置存储
    QMap<QString, QJsonValue> m_configs;        // 配置内容
    QMap<QString, bool> m_configModified;       // 配置是否被修改
    
    // 互斥锁，确保线程安全
    QMutex m_mutex;
};

#endif // CONFIGMANAGER_H 