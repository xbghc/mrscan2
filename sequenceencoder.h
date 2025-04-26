#ifndef SEQUENCEENCODER_H
#define SEQUENCEENCODER_H

#include <QJsonObject>
#include <memory>

class SequenceEncoder
{
public:
    SequenceEncoder();
    static std::unique_ptr<unsigned char[]> encode(QJsonObject &sequence, int &size);
    static std::unique_ptr<unsigned char[]> encodeStop(int id, int &size);
};

#endif // SEQUENCEENCODER_H
