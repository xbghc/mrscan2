#ifndef DEBUGCONFIG_H
#define DEBUGCONFIG_H

#include <QObject>
#include <QString>

namespace config{
    class Debug : public QObject{
        Q_OBJECT
        
    public:
        static constexpr const char* CONFIG_NAME = "Debug";
        static constexpr const char* KEY_MOCK_FILE_PATH = "mock_file_path";
        static constexpr const char* KEY_SCAN_TIME = "scan_time";
        static constexpr const char* KEY_ENABLE_DELAY = "enable_delay";
        static constexpr const char* KEY_LOG_LEVEL = "log_level";
        static constexpr const char* KEY_LOG_TO_FILE = "log_to_file";
        static constexpr const char* KEY_LOG_FILE_PATH = "log_file_path";

        // 单例实例
        static Debug* instance();
        
        // Scanner settings
        static QString mockFilePath();
        static int scanTime();
        static bool enableScanDelay();
        
        static void setMockFilePath(const QString& path);
        static void setScanTime(int seconds);
        static void setEnableScanDelay(bool enable);
        
        // Logging settings
        static int logLevel();
        static bool logToFile();
        static QString logFilePath();
        
        static void setLogLevel(int level);
        static void setLogToFile(bool enable);
        static void setLogFilePath(const QString& path);
        
    signals:
        void mockFilePathChanged(const QString& path);
        void scanTimeChanged(int seconds);
        void enableDelayChanged(bool enable);
        void logLevelChanged(int level);
        void logToFileChanged(bool enable);
        void logFilePathChanged(const QString& path);
        
    private:
        explicit Debug(QObject *parent = nullptr);
    };
}

#endif // DEBUGCONFIG_H
