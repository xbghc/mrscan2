#ifndef VIRTUALSCANNER_H
#define VIRTUALSCANNER_H


class VirtualScanner
{
public:
    VirtualScanner();
    int open();
    int close();
    int write(const unsigned char* buf, size_t len);
    int read(unsigned char* buf, size_t len);
    int ioctl(unsigned char* buf, size_t len);

    void setResult(unsigned char* buf, size_t len);
private:
    unsigned char* _result=nullptr;
    size_t _size=0;
};

#endif // VIRTUALSCANNER_H
