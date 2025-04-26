#include "virtualscanner.h"
#include <cstring>
#include <QThread>

VirtualScanner::VirtualScanner() {
}

VirtualScanner::~VirtualScanner()
{
    if(_result!=nullptr){
        delete[] _result;
    }
}

int VirtualScanner::open()
{
    return 0;
}

int VirtualScanner::close()
{
    return 0;
}

int VirtualScanner::write(const unsigned char *buf, size_t len)
{
    return len;
}

int VirtualScanner::read(unsigned char *buf, size_t len)
{
    if(buf==nullptr){
        return _size;
    }

    QThread::sleep(5);
    if(len >= _size){
        memcpy(buf, _result, _size);
        return _size;
    } else{
        memcpy(buf, _result, len);
        return len;
    }
}

int VirtualScanner::ioctl(unsigned char *buf, size_t len)
{
    return len;
}

void VirtualScanner::setResult(unsigned char *buf, size_t len)
{
    _result = buf;
    _size = len;
}
