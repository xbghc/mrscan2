#ifndef SEQUENCEENCODER_H
#define SEQUENCEENCODER_H

#include <QJsonObject>

class SequenceEncoder
{
public:
    SequenceEncoder();
    static unsigned char *encode(QJsonObject& sequence, int& size);
    static unsigned char *encodeStop(int id, int &size);
};

#endif // SEQUENCEENCODER_H
