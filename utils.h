#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QDebug>

// Log levels
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Macros to simplify logging calls
#define LOG_DEBUG(msg) Logger::log(LogLevel::Debug, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::log(LogLevel::Info, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::log(LogLevel::Warning, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::log(LogLevel::Error, msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::log(LogLevel::Critical, msg, __FILE__, __LINE__)

// Global logger class
class Logger {
public:
    static void log(LogLevel level, const QString& message, const char* file = nullptr, int line = 0);
    static void setLogToFile(bool logToFile, const QString& filePath = "logs/mrscan.log");
    static void setMinLogLevel(LogLevel level);

private:
    static bool s_logToFile;
    static QString s_logFilePath;
    static LogLevel s_minLogLevel;
};

// Global error handling class
class ErrorHandler {
public:
    static void handleError(const QString& message, const QString& details = QString());
    static bool showErrorDialog(const QString& message, const QString& details = QString());
};

QByteArray read(QString filepath);
void newEmptyFile(QFile &file);

#endif // UTILS_H
