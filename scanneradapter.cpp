#include "scanneradapter.h"
#include "virtualscanner.h"
#include "sequenceencoder.h"
#include "sequencevalidator.h"
#include "configmanager.h"

#include <QFile>
#include "utils.h"
#include <memory>

ScannerAdapter::ScannerAdapter(QObject *parent)
    : IScannerAdapter(parent), m_isConnected(false)
{

}

ScannerAdapter::~ScannerAdapter() {
}

int ScannerAdapter::open() {
    int status = scanner.open();
    m_isConnected = (status == 0);
    return status;
}

void ScannerAdapter::scan(QJsonObject sequence) {
    if (!SequenceValidator::validate(sequence)) {
        LOG_ERROR("Invalid scan sequence");
        return;
    }

    int size;
    auto code = SequenceEncoder::encode(sequence, size);
    if (code == nullptr) {
        LOG_ERROR("Sequence encoding failed");
        return;
    }

    // Use ConfigManager to generate ID
    int id = ConfigManager::instance()->generateId();
    if (id < 0) {
        LOG_ERROR("Failed to generate ID");
        return;
    }
    LOG_INFO(QString("Scan started, ID: %1").arg(id));
    memcpy(code.get() + 4, &id, 4);

    if (scanner.write(code.get(), size) != size) {
        LOG_ERROR("Failed to write data to scanner");
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
            LOG_ERROR(QString("Expected size (%1) does not match actual size (%2)").arg(dataSize).arg(totalReceived));
            break;
        }
        totalReceived += curReceived;
    }
    LOG_INFO(QString("Scan completed, ID: %1, Data size: %2").arg(id).arg(totalReceived));
    emit scanEnded(buffer);
}

int ScannerAdapter::stop(int id)
{
    LOG_INFO(QString("Stopping scan, ID: %1").arg(id));
    int size;
    auto code = SequenceEncoder::encodeStop(id, size);
    memcpy(code.get()+4, &id, 4);

    if(scanner.write(code.get(), size) == size){
        emit stoped(id);
        return id;
    }
    LOG_ERROR(QString("Failed to stop scan, ID: %1").arg(id));
    return -1;
}

int ScannerAdapter::close() { 
    m_isConnected = false;
    LOG_INFO("Closing scanner connection");
    return scanner.close(); 
}

