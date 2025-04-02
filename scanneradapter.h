#ifndef SCANNERADAPTER_H
#define SCANNERADAPTER_H

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QThread>
#include <QObject>

#include "virtualscanner.h"

class ScannerAdapter: public QObject
{
    Q_OBJECT
public:
    ScannerAdapter(QObject* parent=nullptr);
    ~ScannerAdapter();
    bool isConnected;
    int open();
    void scan(QJsonObject &sequence);
    int stop(int id);
    int close();
signals:
    void onScanStarted(int id);
    void scanned(QByteArray response);
    void stoped(int id);
private:
    VirtualScanner scanner;
};

#endif // SCANNERADAPTER_H
