#include "virtualscanner.h"
#include <cstring>
#include <QThread>
#include "utils.h"

VirtualScanner::VirtualScanner() : _result(nullptr), _size(0) {
    loadTestData();
}

VirtualScanner::~VirtualScanner() {
    clearResult();
}

void VirtualScanner::loadTestData() {

}

int VirtualScanner::open() {
    LOG_INFO("VirtualScanner: Open called");
    return 0;
}

int VirtualScanner::close() {
    LOG_INFO("VirtualScanner: Close called");
    return 0;
}

int VirtualScanner::write(const unsigned char *buf, size_t len) {
    LOG_INFO(QString("VirtualScanner: Write called, length: %1").arg(len));
    return len;
}

int VirtualScanner::read(unsigned char *buf, size_t len) {
    if (buf == nullptr) {
        return _size;
    }

    if (!_result) {
        LOG_WARNING("VirtualScanner: No data available for reading");
        return 0;
    }

    QThread::sleep(5);
    
    size_t copySize = (len >= _size) ? _size : len;
    memcpy(buf, _result.get(), copySize);
    LOG_INFO(QString("VirtualScanner: Read called, requested: %1 provided: %2").arg(len).arg(copySize));
    return copySize;
}

int VirtualScanner::ioctl(unsigned char *buf, size_t len) {
    LOG_INFO(QString("VirtualScanner: ioctl called, length: %1").arg(len));
    return len;
}

void VirtualScanner::clearResult() {
    _result.reset();
    _size = 0;
}

void VirtualScanner::setResult(unsigned char *buf, size_t len) {
    clearResult();
    
    if (buf != nullptr && len > 0) {
        _result.reset(buf);
        _size = len;
    }
}
