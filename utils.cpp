#include "utils.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>

// Initialize static members
bool Logger::s_logToFile = false;
QString Logger::s_logFilePath = "logs/mrscan.log";
LogLevel Logger::s_minLogLevel = LogLevel::Debug;

void Logger::log(LogLevel level, const QString& message, const char* file, int line)
{
    // Check log level
    if (level < s_minLogLevel) {
        return;
    }

    // Get string representation of log level
    QString levelStr;
    switch (level) {
    case LogLevel::Debug:
        levelStr = "DEBUG";
        break;
    case LogLevel::Info:
        levelStr = "INFO";
        break;
    case LogLevel::Warning:
        levelStr = "WARNING";
        break;
    case LogLevel::Error:
        levelStr = "ERROR";
        break;
    case LogLevel::Critical:
        levelStr = "CRITICAL";
        break;
    }

    // Build log message
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage;
    
    if (file) {
        QString fileName = QString(file).split('/').last();
        logMessage = QString("[%1] [%2] [%3:%4] %5")
            .arg(timestamp)
            .arg(levelStr)
            .arg(fileName)
            .arg(line)
            .arg(message);
    } else {
        logMessage = QString("[%1] [%2] %3")
            .arg(timestamp)
            .arg(levelStr)
            .arg(message);
    }

    // Output to console
    switch (level) {
    case LogLevel::Debug:
        qDebug() << logMessage;
        break;
    case LogLevel::Info:
        qInfo() << logMessage;
        break;
    case LogLevel::Warning:
        qWarning() << logMessage;
        break;
    case LogLevel::Error:
    case LogLevel::Critical:
        qCritical() << logMessage;
        break;
    }

    // Write to log file
    if (s_logToFile) {
        QDir dir;
        dir.mkpath(QFileInfo(s_logFilePath).absolutePath());
        
        QFile file(s_logFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << logMessage << "\n";
            file.close();
        }
    }
}

void Logger::setLogToFile(bool logToFile, const QString& filePath)
{
    s_logToFile = logToFile;
    if (!filePath.isEmpty()) {
        s_logFilePath = filePath;
    }
}

void Logger::setMinLogLevel(LogLevel level)
{
    s_minLogLevel = level;
}

void ErrorHandler::handleError(const QString& message, const QString& details)
{
    LOG_ERROR(message + (details.isEmpty() ? "" : ": " + details));
    showErrorDialog(message, details);
}

bool ErrorHandler::showErrorDialog(const QString& message, const QString& details)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    
    if (!details.isEmpty()) {
        msgBox.setDetailedText(details);
    }
    
    msgBox.setStandardButtons(QMessageBox::Ok);
    return msgBox.exec() == QMessageBox::Ok;
}

QByteArray read(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly)){
        LOG_ERROR(QString("Failed to open file: %1").arg(filepath));
        return nullptr;
    }

    return file.readAll();
}

void newEmptyFile(QFile &file)
{
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Failed to create file: %1").arg(file.fileName()));
    }
    QDataStream out(&file);
    out << (qint32)0;
    file.close();
}
