#include "fakescanner.h"

FakeScanner::FakeScanner() {}

int FakeScanner::open()
{
    return 0;
}

int FakeScanner::close()
{
    return 0;
}

int FakeScanner::write(unsigned char *buf, int len)
{
    return len;
}

int FakeScanner::read(unsigned char *buf, int len)
{
    return len;
}
