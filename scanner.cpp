#include "scanner.h"

#include <QFile>
#include <random>

#include "utils.h"
#include "mrdresponse.h"
#include "configmanager.h"

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

    // 等待扫描结果
    QThread::sleep(5);

    // 返回扫描结果
    QString mockFilePath = ConfigManager::instance()->get("scanner_settings", "MockFilePath").toString();
    if (mockFilePath.isEmpty()) {
        mockFilePath = "D:\\Projects\\QImagesWidget\\data\\20230528103740-T2_TSE-T-3k#1.mrd";
        // LOG_ERROR("MockFilePath not configured in scanner_settings.json. Cannot perform mock scan.");
        // Optionally, emit a failure signal or handle error appropriately
        // For now, let's try a default relative path or fail if read is empty.
        // mockFilePath = "./mock_data/default_scan.mrd"; // Example default
        // For this exercise, if not configured, we'll let file_utils::read return empty.
    }
    
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

