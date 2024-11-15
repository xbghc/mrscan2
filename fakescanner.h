#ifndef FAKESCANNER_H
#define FAKESCANNER_H

class FakeScanner
{
public:
    FakeScanner();
    static int open();
    static int close();
    static int write(unsigned char* buf, int len);
    static int read(unsigned char* buf, int len);
};

#endif // FAKESCANNER_H
