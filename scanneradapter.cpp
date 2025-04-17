#include "scanneradapter.h"
#include "virtualscanner.h"
#include "sequenceencoder.h"
#include "sequencevalidator.h"
#include "configmanager.h"

#include <QFile>
#include "utils.h"

ScannerAdapter::ScannerAdapter(QObject *parent)
    : IScannerAdapter(parent), m_isConnected(false)
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
    m_isConnected = (status == 0);
    return status;
}

void ScannerAdapter::scan(QJsonObject &sequence) {
    if (!SequenceValidator::validate(sequence)) {
        LOG_ERROR("无效的扫描序列");
        return;
    }

    int size;
    unsigned char *code = SequenceEncoder::encode(sequence, size);
    if (code == nullptr) {
        LOG_ERROR("序列编码失败");
        return;
    }

    // 使用ConfigManager生成ID
    int id = ConfigManager::instance()->generateId();
    if (id < 0) {
        LOG_ERROR("生成ID失败");
        return;
    }
    LOG_INFO(QString("扫描开始，ID: %1").arg(id));
    memcpy(code + 4, &id, 4);

    if (scanner.write(code, size) != size) {
        LOG_ERROR("向扫描仪写入数据失败");
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
            LOG_ERROR(QString("预期大小(%1)与实际大小(%2)不符").arg(dataSize).arg(totalReceived));
            break;
        }
        totalReceived += curReceived;
    }
    LOG_INFO(QString("扫描完成，ID: %1，数据大小: %2").arg(id).arg(totalReceived));
    emit scanEnded(buffer);
}

int ScannerAdapter::stop(int id)
{
    LOG_INFO(QString("停止扫描，ID: %1").arg(id));
    int size;
    unsigned char *code = SequenceEncoder::encodeStop(id, size);
    memcpy(code+4, &id, 4);

    if(scanner.write(code, size) == size){
        emit stoped(id);
        return id;
    }
    LOG_ERROR(QString("停止扫描失败，ID: %1").arg(id));
    return -1;
}

int ScannerAdapter::close() { 
    m_isConnected = false;
    LOG_INFO("关闭扫描仪连接");
    return scanner.close(); 
}

