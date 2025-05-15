#include "scanneradapter.h"
#include "virtualscanner.h"
#include "sequenceencoder.h"
#include "configmanager.h"

#include <QFile>
#include "utils.h"
#include <random>

namespace {
}

ScannerAdapter::ScannerAdapter(QObject *parent)
    : IScanner(parent), m_isConnected(false)
{

}

ScannerAdapter::~ScannerAdapter() {
}

int ScannerAdapter::open() {
    m_isConnected = true;
    return 0;
}

void ScannerAdapter::scan(const ExamRequest& request) {
    auto sequence = request.params();

    // Use ConfigManager to generate ID
    auto id = newId();
    LOG_INFO(QString("Scan started, ID: %1").arg(id));

    emit started(id);

    // 等待扫描结果
    QThread::sleep(5);

    // 返回扫描结果
    /// @todo mock文件的路径应该是可配置项
    QByteArray fileContent = FileUtils::read("D:\\Projects\\QImagesWidget\\data\\20230528103740-T2_TSE-T-3k#1.mrd");

    emit completed(new MrdResponse(fileContent));
}

QString ScannerAdapter::stop(QString id)
{
    LOG_INFO(QString("Stopping scan, ID: %1").arg(id));
    int size;
    auto code = SequenceEncoder::encodeStop(id.toInt(), size);
    
    LOG_ERROR(QString("Failed to stop scan, ID: %1").arg(id));
    return "-1";
}

int ScannerAdapter::close() {
    LOG_INFO(QString("Scanner closed"));

    m_isConnected = false;
    return 0;
}

QString ScannerAdapter::newId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    return QString::number(dist(gen));
}

