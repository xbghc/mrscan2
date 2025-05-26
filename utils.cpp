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
    msgBox.setWindowTitle(QObject::tr("Error"));
    msgBox.setText(message);
    
    if (!details.isEmpty()) {
        msgBox.setDetailedText(details);
    }
    
    msgBox.setStandardButtons(QMessageBox::Ok);
    return msgBox.exec() == QMessageBox::Ok;
}

QString json_utils::get(const QJsonObject &obj, const QString key, QString d)
{
    if(obj.contains(key)) {
        return obj[key].toString();
    }
    return d;
}

int json_utils::get(const QJsonObject &obj, const QString key, int d)
{
    if(obj.contains(key)){
        return obj[key].toInt();
    }
    return d;
}

QJsonDocument json_utils::readFromFile(const QString &fpath)
{
    auto bytes = file_utils::read(fpath);
    return QJsonDocument::fromJson(bytes);
}


void json_utils::saveToFile(const QString &fpath, QJsonObject obj)
{
    auto doc = QJsonDocument(obj);
    file_utils::save(fpath, doc.toJson());
}

void json_utils::saveToFile(const QString &fpath, QJsonArray array)
{
    auto doc = QJsonDocument(array);
    file_utils::save(fpath, doc.toJson());
}

QString utils::secondsToString(int seconds){
    return QString("%1:%2").arg(seconds/60).arg(seconds%60, 2, 10, 0, QChar(u'0'));
}
