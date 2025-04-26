#include "configmanager.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>
#include <random>


ConfigManager* ConfigManager::instance()
{
    // 使用局部静态变量确保线程安全的初始化（C++11保证）
    static ConfigManager s_instance;
    return &s_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    // Ensure config directory exists
    QDir configDir("./configs");
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            LOG_ERROR("Failed to create config directory");
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
    QJsonValue configValue;
    bool success = false;
    
    if (!file.exists()) {
        LOG_WARNING(QString("Config file does not exist: %1").arg(filePath));
        
        QMutexLocker locker(&m_mutex);
        m_configs[configName] = QJsonValue();
        m_configModified[configName] = false;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("Cannot open config file: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        LOG_ERROR(QString("Config file format is incorrect: %1").arg(filePath));
        return false;
    }
    
    if (doc.isObject()) {
        configValue = doc.object();
        success = true;
    } else if (doc.isArray()) {
        configValue = doc.array();
        success = true;
    } else {
        LOG_ERROR(QString("Config file content is not a valid JSON object or array: %1").arg(filePath));
        return false;
    }
    
    if (success) {
        QMutexLocker locker(&m_mutex);
        m_configs[configName] = configValue;
        m_configModified[configName] = false;
    }
    
    return success;
}

bool ConfigManager::saveConfig(const QString& configName)
{
    QJsonValue configValue;
    bool exists = false;
    bool modified = false;
    
    {
        QMutexLocker locker(&m_mutex);
        exists = m_configs.contains(configName);
        if (exists) {
            configValue = m_configs[configName];
            modified = m_configModified[configName];
        }
    }
    
    if (!exists) {
        LOG_WARNING(QString("Attempting to save non-existent config: %1").arg(configName));
        return false;
    }
    
    if (!modified) {
        return true;
    }
    
    QString filePath = getConfigFilePath(configName);
    QJsonDocument doc;
    
    if (configValue.isObject()) {
        doc = QJsonDocument(configValue.toObject());
    } else if (configValue.isArray()) {
        doc = QJsonDocument(configValue.toArray());
    } else {
        LOG_ERROR(QString("Config is not a valid JSON object or array: %1").arg(configName));
        return false;
    }

    // Ensure directory exists
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        LOG_ERROR(QString("Failed to create directory for config file: %1").arg(dir.path()));
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Cannot write to config file: %1").arg(filePath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    {
        QMutexLocker locker(&m_mutex);
        if (m_configs.contains(configName)) {
            m_configModified[configName] = false;
        }
    }
    
    LOG_INFO(QString("Config saved: %1").arg(configName));
    
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
    bool needLoad = false;
    
    {
        QMutexLocker locker(&m_mutex);
        configExists = m_configs.contains(configName);
        if (configExists) {
            configValue = m_configs[configName];
        } else {
            needLoad = true;
        }
    }
    
    if (needLoad) {
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
    if (path.isEmpty()) {
        LOG_ERROR("Cannot set config value: empty path");
        return false;
    }
    
    bool configNeedsLoading = false;
    {
        QMutexLocker locker(&m_mutex);
        configNeedsLoading = !m_configs.contains(configName);
    }
    
    if (configNeedsLoading) {
        if (!loadConfig(configName)) {
            QMutexLocker locker(&m_mutex);
            m_configs[configName] = QJsonObject();
            m_configModified[configName] = false;
        }
    }
    
    QMutexLocker locker(&m_mutex);
    QJsonValue& root = m_configs[configName];
    QStringList pathParts = parsePath(path);
    
    bool success = setValueAtPath(root, pathParts, value);
    if (success) {
        m_configModified[configName] = true;
        locker.unlock();
        emit configChanged(configName, path);
    }
    
    return success;
}

bool ConfigManager::setObject(const QString& configName, const QString& path, const QJsonObject& obj)
{
    return setValue(configName, path, obj);
}

bool ConfigManager::setArray(const QString& configName, const QString& path, const QJsonArray& array)
{
    return setValue(configName, path, array);
}

int ConfigManager::generateId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    return dist(gen);
}

QString ConfigManager::getConfigFilePath(const QString& configName)
{
    return QString("./configs/%1.json").arg(configName);
}

QStringList ConfigManager::parsePath(const QString& path)
{
    return path.split(".", Qt::SkipEmptyParts);
}

QJsonValue ConfigManager::findValue(const QJsonValue& root, const QStringList& pathParts)
{
    if (pathParts.isEmpty()) {
        return root;
    }
    
    QJsonValue current = root;
    
    for (const QString& part : pathParts) {
        if (current.isObject()) {
            if (!current.toObject().contains(part)) {
                return QJsonValue();
            }
            current = current.toObject().value(part);
        } else if (current.isArray() && part.toInt() < current.toArray().size()) {
            bool ok;
            int index = part.toInt(&ok);
            if (!ok || index < 0 || index >= current.toArray().size()) {
                return QJsonValue();
            }
            current = current.toArray().at(index);
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
    
    // For root element, we need special handling
    if (!root.isObject() && !root.isArray()) {
        root = QJsonObject();
    }
    
    // 使用不同的方法导航JSON树，避免直接引用QJsonValueRef
    if (pathParts.size() == 1) {
        // 只有一层路径，直接设置值
        QString part = pathParts.first();
        
        if (root.isObject()) {
            QJsonObject obj = root.toObject();
            obj[part] = value;
            root = obj;
            return true;
        } else if (root.isArray()) {
            bool ok;
            int index = part.toInt(&ok);
            if (!ok || index < 0) {
                LOG_ERROR(QString("Invalid array index in path: %1").arg(part));
                return false;
            }
            
            QJsonArray arr = root.toArray();
            
            // Extend array if needed
            while (arr.size() <= index) {
                arr.append(QJsonValue());
            }
            
            arr[index] = value;
            root = arr;
            return true;
        } else {
            LOG_ERROR("Cannot set value: root is neither object nor array");
            return false;
        }
    }
    
    // 处理多层路径的情况
    QString firstPart = pathParts.first();
    QStringList remainingParts = pathParts.mid(1);
    
    if (root.isObject()) {
        QJsonObject obj = root.toObject();
        QJsonValue nextValue = obj.contains(firstPart) ? obj[firstPart] : QJsonValue(QJsonObject());
        
        if (setValueAtPath(nextValue, remainingParts, value)) {
            obj[firstPart] = nextValue;
            root = obj;
            return true;
        }
        return false;
    } else if (root.isArray()) {
        bool ok;
        int index = firstPart.toInt(&ok);
        if (!ok || index < 0) {
            LOG_ERROR(QString("Invalid array index in path: %1").arg(firstPart));
            return false;
        }
        
        QJsonArray arr = root.toArray();
        
        // Extend array if needed
        while (arr.size() <= index) {
            arr.append(QJsonValue());
        }
        
        QJsonValue nextValue = arr[index];
        
        if (setValueAtPath(nextValue, remainingParts, value)) {
            arr[index] = nextValue;
            root = arr;
            return true;
        }
        return false;
    } else {
        LOG_ERROR("Cannot navigate path: node is neither object nor array");
        return false;
    }
} 
