#include "scanner.h"

#include <QFile>
#include <random>
#include <QThread>

#include "utils.h"
#include "mrdresponse.h"
#include "configmanager.h"
#include "debugconfig.h"

namespace {
}

VScanner::VScanner(QObject *parent)
    : IScanner(parent), m_isConnected(false)
{

}

VScanner::~VScanner() {
}

int VScanner::open() {
    m_isConnected = true;
    return 0;
}

void VScanner::scan(const ExamRequest& request) {
    auto sequence = request.params();

    // Use ConfigManager to generate ID
    auto id = newId();
    LOG_INFO(QString("Scan started, ID: %1").arg(id));

    emit started(id);

    // 模拟扫描延时
    if (config::Debug::enableScanDelay()) {
        int scanTime = config::Debug::scanTime();
        LOG_INFO(QString("Simulating scan delay: %1 seconds").arg(scanTime));
        QThread::sleep(scanTime);
    }

    // 返回扫描结果
    QString mockFilePath = config::Debug::mockFilePath();
    
    QByteArray fileContent = file_utils::read(mockFilePath);
    if (fileContent.isEmpty() && !mockFilePath.isEmpty()) {
        LOG_ERROR(QString("Failed to read mock file from path: %1").arg(mockFilePath));
        // Handle error: maybe emit a completed signal with an error response
    } else if (fileContent.isEmpty() && mockFilePath.isEmpty()) {
        LOG_ERROR("Mock file path is empty and not configured. Cannot load mock data.");
    }

    emit completed(new MrdResponse(fileContent));
}

/// @todo 应该中止扫描
void VScanner::stop(QString id)
{
    LOG_INFO(QString("Stopping scan, ID: %1").arg(id));
    emit stoped(id);
}

int VScanner::close() {
    LOG_INFO(QString("Scanner closed"));

    m_isConnected = false;
    return 0;
}

QString VScanner::newId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    return QString::number(dist(gen));
}

