#include "scanneradapter.h"
#include "virtualscanner.h"
#include "sequenceencoder.h"
#include "sequencevalidator.h"

#include <QFile>
#include "QImagesWidget/utils.h"

namespace {

int generateId() {
    const static QString kPath = "./configs/id.txt";

    QFile file(kPath);
    if (!file.exists()) {
        newEmptyFile(file);
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
    : QObject(parent)
{
    auto fileContent = read("D:\\Projects\\QImagesWidget\\data\\20230528103740-T2_TSE-T-3k#1.mrd");

    auto len = fileContent.length();
    auto buffer = new unsigned char[len];
    memcpy(buffer, fileContent.constData(), len);

    scanner.setResult(buffer, len);
}

ScannerAdapter::~ScannerAdapter() {
}

int ScannerAdapter::open() {
    int status = scanner.open();
    return status;
}

void ScannerAdapter::scan(QJsonObject &sequence) {
    if (!SequenceValidator::validate(sequence)) {
        qDebug() << "invalidate sequence";
        return;
    }

    int size;
    unsigned char *code = SequenceEncoder::encode(sequence, size);
    if (code == nullptr) {
        qDebug() << "failed to encode sequence";
        return;
    }

    int id = generateId();
    if (id < 0) {
        qDebug() << "failed to generate id";
        return;
    }
    memcpy(code + 4, &id, 4);

    if (scanner.write(code, size) != size) {
        qDebug() << "failed to write to scanner";
        return;
    }

    emit scanStarted(id);

    int dataSize = scanner.read(nullptr, 0);
    QByteArray buffer;
    buffer.resize(dataSize);

    int totalReceived=0, curReceived=0;
    while(totalReceived<dataSize){
        curReceived = scanner.read(reinterpret_cast<unsigned char*>(buffer.data()), dataSize-totalReceived);
        if(curReceived == 0){
            qDebug() << QString("预期大小(%1)与实际大小不符(%)").arg(dataSize, totalReceived);
            break;
        }
        totalReceived += curReceived;
    }
    emit scanEnded(buffer);
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

