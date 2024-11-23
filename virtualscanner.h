#ifndef VIRTUALSCANNER_H
#define VIRTUALSCANNER_H

#include <vector>

class VirtualScanner
{
public:
    VirtualScanner();
    int open();
    int close();
    int write(const unsigned char* buf, int len);
    int read(unsigned char* buf, int len);

private:
    std::vector<unsigned char>content;
    size_t position=0;
    void cleanup(){
        content.clear();
        position = 0;
    }
    int scanningId;
    int stop(int id, int implement);
    int tune(int id, int implement, int dataSize, const unsigned char* data);
    int rfopt(int id, int implement, int dataSize, const unsigned char* data);
    int shimming(int id, int implement, int dataSize, const unsigned char* data);
    int t1(int id, int implement, int dataSize, const unsigned char* data);
    int t2(int id, int implement, int dataSize, const unsigned char* data);
};

#endif // VIRTUALSCANNER_H
