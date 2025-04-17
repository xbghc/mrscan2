#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QString>
#include <QFile>

QByteArray read(QString filepath);
void newEmptyFile(QFile &file);

#endif // UTILS_H
