#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QList>

class Sequence
{
public:
    Sequence();
    static Sequence fromName();

private:
    static QList<Sequence> availableSequences;
};

#endif // SEQUENCE_H
