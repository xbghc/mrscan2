#ifndef SCANNERADAPTER_H
#define SCANNERADAPTER_H

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QThread>
#include <QObject>

#include "fakescanner.h"

class ScannerAdapter: public QObject
{
    Q_OBJECT
public:
    ScannerAdapter(QObject* parent=nullptr);
    ~ScannerAdapter();
    bool isConnected;
    int open();
    int scan(QJsonObject &sequence);
    int stop(int id);
    int close();
signals:
    void scanned(QByteArray response);
    void stoped(int id);
private:
    const static int32_t kResponseHeaderSize=16;
    static const size_t kDefaultBufferSize = 32 * 1024 * 1024; // 32MB

    void listen();
    bool listenerReadHeader(unsigned char* headerBuf, int& dataSize);
    bool ensureBufferCapacity(size_t required);

    QThread* listenThread;
    FakeScanner scanner;
    bool shouldStop=false;
    std::unique_ptr<uint8_t[]> responseBuf;
    size_t responseBufCapacity = kDefaultBufferSize;
};

#endif // SCANNERADAPTER_H
