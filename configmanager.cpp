#include "configmanager.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>

// 初始化静态变量
ConfigManager* ConfigManager::s_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    // 确保配置目录存在
    QDir configDir("./configs");
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            LOG_ERROR("无法创建配置目录");
        }
    }
}

ConfigManager::~ConfigManager()
{
    saveAllModified();
}

bool ConfigManager::loadConfig(const QString& configName)
{
    QString filePath = getConfigFilePath(configName);
    QFile file(filePath);
    
    if (!file.exists()) {
        LOG_WARNING(QString("配置文件不存在: %1").arg(filePath));
        
        QMutexLocker locker(&m_mutex);
        m_configs[configName] = QJsonValue();
        m_configModified[configName] = false;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开配置文件: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        LOG_ERROR(QString("配置文件格式不正确: %1").arg(filePath));
        return false;
    }
    
    QJsonValue configValue;
    if (doc.isObject()) {
        configValue = doc.object();
    } else if (doc.isArray()) {
        configValue = doc.array();
    } else {
        LOG_ERROR(QString("配置文件内容不是有效的JSON对象或数组: %1").arg(filePath));
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    m_configs[configName] = configValue;
    m_configModified[configName] = false;
    return true;
}

bool ConfigManager::saveConfig(const QString& configName)
{
    QJsonValue configValue;
    bool exists;
    
    {
        QMutexLocker locker(&m_mutex);
        exists = m_configs.contains(configName);
        if (exists) {
            configValue = m_configs[configName];
        }
    }
    
    if (!exists) {
        LOG_WARNING(QString("尝试保存不存在的配置: %1").arg(configName));
        return false;
    }
    
    QString filePath = getConfigFilePath(configName);
    QJsonDocument doc;
    
    if (configValue.isObject()) {
        doc = QJsonDocument(configValue.toObject());
    } else if (configValue.isArray()) {
        doc = QJsonDocument(configValue.toArray());
    } else {
        LOG_ERROR(QString("配置不是有效的JSON对象或数组: %1").arg(configName));
        return false;
    }
    
    // 文件操作在锁之外进行
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("无法写入配置文件: %1").arg(filePath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    {
        QMutexLocker locker(&m_mutex);
        m_configModified[configName] = false;
    }
    
    LOG_INFO(QString("配置已保存: %1").arg(configName));
    
    return true;
}

bool ConfigManager::saveAllModified()
{
    QStringList configsToSave;
    
    {
        QMutexLocker locker(&m_mutex);
        for (auto it = m_configModified.begin(); it != m_configModified.end(); ++it) {
            if (it.value()) {
                configsToSave.append(it.key());
            }
        }
    }
    
    bool allSaved = true;
    for (const QString& configName : configsToSave) {
        bool saved = saveConfig(configName);
        if (!saved) {
            allSaved = false;
        }
    }
    
    return allSaved;
}

QJsonValue ConfigManager::getValue(const QString& configName, const QString& path)
{
    QJsonValue configValue;
    bool configExists = false;
    
    {
        QMutexLocker locker(&m_mutex);
        configExists = m_configs.contains(configName);
        if (configExists) {
            configValue = m_configs[configName];
        }
    }
    
    if (!configExists) {
        // 锁外加载配置
        if (!loadConfig(configName)) {
            return QJsonValue();
        }
        
        QMutexLocker locker(&m_mutex);
        configValue = m_configs[configName];
    }
    
    if (path.isEmpty()) {
        return configValue;
    }
    
    QStringList pathParts = parsePath(path);
    return findValue(configValue, pathParts);
}

QJsonObject ConfigManager::getObject(const QString& configName, const QString& path)
{
    QJsonValue value = getValue(configName, path);
    if (!value.isObject()) {
        return QJsonObject();
    }
    return value.toObject();
}

QJsonArray ConfigManager::getArray(const QString& configName, const QString& path)
{
    QJsonValue value = getValue(configName, path);
    if (!value.isArray()) {
        return QJsonArray();
    }
    return value.toArray();
}

bool ConfigManager::setValue(const QString& configName, const QString& path, const QJsonValue& value)
{
    QJsonValue configValue;
    bool configExists = false;
    
    {
        QMutexLocker locker(&m_mutex);
        configExists = m_configs.contains(configName);
        if (configExists) {
            configValue = m_configs[configName];
        }
    }
    
    if (!configExists) {
        // 锁外加载配置
        if (!loadConfig(configName)) {
            // 加载失败，创建新的空配置
            configValue = path.isEmpty() ? value : QJsonObject();
        } else {
            QMutexLocker locker(&m_mutex);
            configValue = m_configs[configName];
        }
    }
    
    if (path.isEmpty()) {
        QMutexLocker locker(&m_mutex);
        m_configs[configName] = value;
        m_configModified[configName] = true;
        emit configChanged(configName, path);
        return true;
    }
    
    QStringList pathParts = parsePath(path);
    
    // 在修改前复制一份
    QJsonValue updatedValue = configValue;
    bool success = setValueAtPath(updatedValue, pathParts, value);
    
    if (success) {
        QMutexLocker locker(&m_mutex);
        m_configs[configName] = updatedValue;
        m_configModified[configName] = true;
        emit configChanged(configName, path);
    }
    
    return success;
}

bool ConfigManager::setObject(const QString& configName, const QString& path, const QJsonObject& obj)
{
    return setValue(configName, path, QJsonValue(obj));
}

bool ConfigManager::setArray(const QString& configName, const QString& path, const QJsonArray& array)
{
    return setValue(configName, path, QJsonValue(array));
}

int ConfigManager::generateId()
{
    int currentId;
    
    {
        currentId = getValue("ids", "nextId").toInt();
        
        if (currentId <= 0) {
            QMutexLocker locker(&m_mutex);
            currentId = 1;
        }
    }
    
    // 更新并保存新ID
    setValue("ids", "nextId", currentId + 1);
    saveConfig("ids");
    
    return currentId;
}

QString ConfigManager::getConfigFilePath(const QString& configName)
{
    return QString("./configs/%1.json").arg(configName);
}

QStringList ConfigManager::parsePath(const QString& path)
{
    return path.split(".");
}

QJsonValue ConfigManager::findValue(const QJsonValue& root, const QStringList& pathParts)
{
    QJsonValue current = root;
    
    for (const QString& part : pathParts) {
        if (current.isObject()) {
            if (!current.toObject().contains(part)) {
                return QJsonValue();
            }
            current = current.toObject()[part];
        } else if (current.isArray()) {
            bool ok;
            int index = part.toInt(&ok);
            if (!ok || index < 0 || index >= current.toArray().size()) {
                return QJsonValue();
            }
            current = current.toArray()[index];
        } else {
            return QJsonValue();
        }
    }
    
    return current;
}

bool ConfigManager::setValueAtPath(QJsonValue& root, const QStringList& pathParts, const QJsonValue& value)
{
    if (pathParts.isEmpty()) {
        root = value;
        return true;
    }
    
    QString currentPart = pathParts.first();
    QStringList remainingParts = pathParts.mid(1);
    
    if (root.isObject()) {
        QJsonObject obj = root.toObject();
        
        if (remainingParts.isEmpty()) {
            obj[currentPart] = value;
            root = obj;
            return true;
        } else {
            QJsonValue nestedValue = obj.contains(currentPart) ? obj[currentPart] : QJsonValue(QJsonObject());
            if (setValueAtPath(nestedValue, remainingParts, value)) {
                obj[currentPart] = nestedValue;
                root = obj;
                return true;
            }
        }
    } else if (root.isArray()) {
        QJsonArray arr = root.toArray();
        
        bool ok;
        int index = currentPart.toInt(&ok);
        
        if (!ok || index < 0) {
            return false;
        }
        
        // 如果需要扩展数组
        while (arr.size() <= index) {
            arr.append(QJsonValue());
        }
        
        if (remainingParts.isEmpty()) {
            arr[index] = value;
            root = arr;
            return true;
        } else {
            QJsonValue nestedValue = arr[index];
            if (nestedValue.isNull()) {
                nestedValue = QJsonObject();
            }
            
            if (setValueAtPath(nestedValue, remainingParts, value)) {
                arr[index] = nestedValue;
                root = arr;
                return true;
            }
        }
    } else if (root.isNull() || root.isUndefined()) {
        // 如果当前节点为空，需要创建新对象
        bool isNumber;
        remainingParts.first().toInt(&isNumber);
        
        if (isNumber) {
            // 下一级是数组
            root = QJsonArray();
        } else {
            // 下一级是对象
            root = QJsonObject();
        }
        
        return setValueAtPath(root, pathParts, value);
    }
    
    return false;
} 
