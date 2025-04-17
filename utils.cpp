#include "utils.h"

#include <QDebug>
#include <QFile>

QByteArray read(QString filepath)
{
    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Faild to open file: " << filepath;
        return nullptr;
    }

    return file.readAll();
}

void newEmptyFile(QFile &file)
{
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "failed to create file: " << file.fileName();
    }
    QDataStream out(&file);
    out << (qint32)0;
    file.close();
}
