#include "utils.h"

#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

namespace file_utils{
/// @todo
int loadMrdFiles(){
    return 0;
}

QByteArray read(const QString& fpath){
    QFile file(fpath);

    if(!file.open(QIODevice::ReadOnly)){
        LOG_ERROR(QString("Failed to open file:%1 Error: %2").arg(fpath, file.errorString()));
        return QByteArray();
    }
    auto data = file.readAll();
    return data;
}

void save(const QString& fpath, QByteArray content){
    QFile file(fpath);

    if(!file.open(QIODevice::WriteOnly)){
        LOG_ERROR(QString("Failed to open file:%1 Error: %2").arg(fpath, file.errorString()));
        return;
    }

    file.write(content);
}

// Get all channel files from the given path
QStringList getAllChannelsFile(const QString& path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        LOG_WARNING(QString("File path does not exist: %1").arg(path));
        return {};
    }

    QDir dir = fileInfo.absoluteDir();
    QString fileName = fileInfo.fileName();

    static QRegularExpression namePattern("^(.*)#(\\d+)\\.(\\w+)$");
    QRegularExpressionMatch match = namePattern.match(fileName);
    if (!match.hasMatch()) {
        LOG_WARNING("Invalid filename format, expected format: prefix#number.suffix");
        return {};
    }

    QString prefix = QRegularExpression::escape(match.captured(1));
    QString suffix = QRegularExpression::escape(match.captured(3));

    // Build new regex to match related files
    QString pattern = QString("^%1#\\d+\\.%2$").arg(prefix, suffix);
    QRegularExpression regex(pattern);

    QStringList result;
    const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
    for (const QString& file : files) {
        if (regex.match(file).hasMatch()) {
            result.append(dir.filePath(file));
        }
    }

    return result;
}

} // namespace file_utils
