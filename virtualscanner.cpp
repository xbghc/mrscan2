#include "virtualscanner.h"
#include <cstring>


VirtualScanner::VirtualScanner() {
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
    } else if(len >= _size){
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
