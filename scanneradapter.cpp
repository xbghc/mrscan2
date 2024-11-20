#include "scanneradapter.h"
#include "fakescanner.h"
#include "sequenceencoder.h"
#include "sequencevalidator.h"

#include <QFile>

namespace {

int generateId() {
    const static QString kPath = "./configs/id.txt";

    QFile file(kPath);
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to create file: " << kPath;
            return -1;
        }
        QDataStream out(&file);
        out << (qint32)0;
        file.close();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "failed to open file: " << kPath;
        return -1;
    }

    QDataStream in(&file);
    qint32 id;
    in >> id;
    file.close();

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "failed to write nextId to file: " << kPath;
        return -1;
    }
    QDataStream out(&file);
    out << (qint32)(id + 1);
    file.close();

    return id;
}

} // namespace

ScannerAdapter::ScannerAdapter(QObject *parent)
    : QObject(parent), listenThread(nullptr) {
    responseBuf.reset(new uint8_t[responseBufCapacity]);
}

ScannerAdapter::~ScannerAdapter() {
    shouldStop = true;
    if(listenThread != nullptr){
        listenThread->wait();
    }
}

int ScannerAdapter::open() {
    int status = scanner.open();
    if (status >= 0) {
        listenThread = QThread::create([this]() { this->listen(); });
        connect(listenThread, &QThread::finished, listenThread,
                &QThread::deleteLater);
        listenThread->start();
    }
    return status;
}

int ScannerAdapter::scan(QJsonObject &sequence) {
    if (!SequenceValidator::validate(sequence)) {
        qDebug() << "invalidate sequence";
        return -1;
    }

    int size;
    unsigned char *code = SequenceEncoder::encode(sequence, size);
    if (code == nullptr) {
        qDebug() << "failed to encode sequence";
        return -1;
    }

    int id = generateId();
    if (id < 0) {
        qDebug() << "failed to generate id";
        return -1;
    }
    memcpy(code + 4, &id, 4);

    if (scanner.write(code, size) != size) {
        qDebug() << "failed to write to scanner";
        return -1;
    }

    return id;
}

int ScannerAdapter::stop(int id)
{
    int size;
    unsigned char *code = SequenceEncoder::encodeStop(id, size);
    memcpy(code+4, &id, 4);

    if(scanner.write(code, size) == size){
        emit stoped(id);
        return id;
    }
    return -1;
}

int ScannerAdapter::close() { return scanner.close(); }

void ScannerAdapter::listen() {

    uint8_t headerBuf[kResponseHeaderSize];
    while(!shouldStop){
        QThread::msleep(200);

        int dataSize;
        while(!listenerReadHeader(headerBuf, dataSize)){
            continue;
        }

        size_t totalSize = kResponseHeaderSize + dataSize;
        if(!ensureBufferCapacity(totalSize)){
            return;
        }

        memcpy(responseBuf.get(), headerBuf, kResponseHeaderSize);

        size_t totalRead = kResponseHeaderSize;
        while(totalRead < totalSize){
            int readSize = scanner.read(responseBuf.get() + totalRead, totalSize - totalRead);
            if(readSize <= 0){
                qDebug() << "error happened";
                return;
            }
            totalRead += readSize;
        }

        QByteArray responseData(reinterpret_cast<const char*>(responseBuf.get()), totalSize);

        emit scanned(responseData);
    }
}

bool ScannerAdapter::listenerReadHeader(unsigned char *headerBuf, int& dataSize)
{
    int size =scanner.read(headerBuf, kResponseHeaderSize);

    if(size <= 0){
        return false;
    }

    if(size != 16){
        qDebug() << "unexpeted headerSize";
        return false;
    }

    static const int kResponseDataSizeOffset = 12;
    dataSize = *reinterpret_cast<int32_t *>
               (headerBuf + kResponseDataSizeOffset);

    return true;
}

bool ScannerAdapter::ensureBufferCapacity(size_t required)
{
    if(required > responseBufCapacity){
        try{
            responseBufCapacity = required;
            responseBuf.reset(new uint8_t[responseBufCapacity]);
        }catch(const std::bad_alloc&){
            return false;
        }
    }

    return true;
}
