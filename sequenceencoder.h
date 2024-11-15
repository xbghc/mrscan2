#ifndef SEQUENCEENCODER_H
#define SEQUENCEENCODER_H

#include <QJsonObject>

class SequenceEncoder
{
public:
    SequenceEncoder();
    static const unsigned char *encode(QJsonObject& sequence, int& size);
};

#endif // SEQUENCEENCODER_H
