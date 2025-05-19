#include "scanner.h"

#include <QFile>
#include <random>

#include "utils.h"
#include "mrdresponse.h"

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
    /// @todo mock文件的路径应该是可配置项
    QByteArray fileContent = file_utils::read("D:\\Projects\\QImagesWidget\\data\\20230528103740-T2_TSE-T-3k#1.mrd");

    emit completed(new MrdResponse(fileContent));
}

/// @todo 应该让scan中的代码不再返回
QString VScanner::stop(QString id)
{
    LOG_INFO(QString("Stopping scan, ID: %1").arg(id));
    return id;
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

