#include "debugconfig.h"
#include "configmanager.h"
#include "utils.h"

namespace config{

// 单例模式实现
Debug* Debug::instance() {
    static Debug s_instance;
    return &s_instance;
}

// 构造函数
Debug::Debug(QObject *parent) : QObject(parent) {
}

// Scanner settings implementation
QString Debug::mockFilePath(){
    auto cm = ConfigManager::instance();
    auto path = cm->get(CONFIG_NAME, KEY_MOCK_FILE_PATH);
    if(path.isNull()){
        QString defaultPath = "D:\\Projects\\QImagesWidget\\data\\20230528103740-T2_TSE-T-3k#1.mrd";
        cm->set(CONFIG_NAME, KEY_MOCK_FILE_PATH, defaultPath);
        return defaultPath;
    }
    return path.toString();
}

int Debug::scanTime(){
    auto cm = ConfigManager::instance();
    auto time = cm->get(CONFIG_NAME, KEY_SCAN_TIME);
    if(time.isNull()){
        cm->set(CONFIG_NAME, KEY_SCAN_TIME, 5);
        return 5;
    }
    return time.toInt();
}

bool Debug::enableScanDelay(){
    auto cm = ConfigManager::instance();
    auto enable = cm->get(CONFIG_NAME, KEY_ENABLE_DELAY);
    if(enable.isNull()){
        cm->set(CONFIG_NAME, KEY_ENABLE_DELAY, false);
        return false;
    }
    return enable.toBool();
}

void Debug::setMockFilePath(const QString& path){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_MOCK_FILE_PATH, path);
    
    // 发射信号
    emit instance()->mockFilePathChanged(path);
}

void Debug::setScanTime(int seconds){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_SCAN_TIME, seconds);
    
    // 发射信号
    emit instance()->scanTimeChanged(seconds);
}

void Debug::setEnableScanDelay(bool enable){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_ENABLE_DELAY, enable);
    
    // 发射信号
    emit instance()->enableDelayChanged(enable);
}

// Logging settings implementation
int Debug::logLevel(){
    auto cm = ConfigManager::instance();
    auto level = cm->get(CONFIG_NAME, KEY_LOG_LEVEL);
    if(level.isNull()){
        cm->set(CONFIG_NAME, KEY_LOG_LEVEL, 0); // Debug level
        return 0;
    }
    return level.toInt();
}

bool Debug::logToFile(){
    auto cm = ConfigManager::instance();
    auto enable = cm->get(CONFIG_NAME, KEY_LOG_TO_FILE);
    if(enable.isNull()){
        cm->set(CONFIG_NAME, KEY_LOG_TO_FILE, true);
        return true;
    }
    return enable.toBool();
}

QString Debug::logFilePath(){
    auto cm = ConfigManager::instance();
    auto path = cm->get(CONFIG_NAME, KEY_LOG_FILE_PATH);
    if(path.isNull()){
        QString defaultPath = "logs/mrscan.log";
        cm->set(CONFIG_NAME, KEY_LOG_FILE_PATH, defaultPath);
        return defaultPath;
    }
    return path.toString();
}

void Debug::setLogLevel(int level){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_LOG_LEVEL, level);
    
    // 发射信号
    emit instance()->logLevelChanged(level);
}

void Debug::setLogToFile(bool enable){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_LOG_TO_FILE, enable);
    
    // 发射信号
    emit instance()->logToFileChanged(enable);
}

void Debug::setLogFilePath(const QString& path){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_LOG_FILE_PATH, path);
    
    // 发射信号
    emit instance()->logFilePathChanged(path);
}

} // namespace config
