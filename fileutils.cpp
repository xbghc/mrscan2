#include "utils.h"

#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

namespace file_utils{

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

} // namespace file_utils
