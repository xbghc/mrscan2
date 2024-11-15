#ifndef SCANNERADAPTER_H
#define SCANNERADAPTER_H

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QObject>

class ScannerAdapter {
public:
    ScannerAdapter();
    static bool isConnected;
    static int open();
    static void scan(QJsonObject &sequence, std::function<void(QList<QImage>, QList<QJsonObject>)>);
    static int close();
};

#endif // SCANNERADAPTER_H
