#include "sequenceencoder.h"

#include <QJsonArray>

namespace {
static_assert(sizeof(int32_t) == 4, "uint32_t must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");

void setHeader(unsigned char* buf, char type, char implement, int size){
    buf[0] = 'N';
    buf[1] = 'M';
    buf[2] = 'R';
    buf[3] = 1;

    // id will be initialized before write to scanner

    // sequence identify
    buf[8] = 0;
    buf[9] = 0;
    buf[10] = type;
    buf[11] = implement;

    // size
    memcpy(buf+12, &size, 4);
}

unsigned char *encodeTune(QJsonObject &sequence, int&size, char implement=1){
    size = 16;
    unsigned char *out = new unsigned char[size];
    setHeader(out, 2, 1, size-16);
    return out;
}

unsigned char *encodeRfopt(QJsonObject &sequence, int&size, char implement=1){
    size = 24;
    unsigned char *out = new unsigned char[size];
    int observeFrequency = sequence["observeFrequency"].toInt();
    float power = sequence["power"].toDouble();
    memcpy(out + 16, &observeFrequency, 4);
    memcpy(out + 20, &power, 4);

    setHeader(out, 3, 1, size-16);
    return out;
}

unsigned char *encodeShim(QJsonObject &sequence, int&size, char implement=1){
    size = 20;
    unsigned char *out = new unsigned char[size];
    int observeFrequency = sequence["observeFrequency"].toInt();
    memcpy(out + 16, &observeFrequency, 4);

    setHeader(out, 4, 1, size-16);
    return out;
}

unsigned char *encodeT1(QJsonObject &sequence, int &size, char implement=1) {
    QJsonObject parameters = sequence["parameters"].toObject();
    int observeFrequency = parameters["observeFrequency"].toInt();
    int32_t noSamples = parameters["noSamples"].toInt();
    int32_t noViews = parameters["noViews"].toInt();
    int32_t noViews2 = parameters["noViews2"].toInt();
    int32_t sliceThickness = parameters["sliceThickness"].toInt();
    int32_t sliceSeparation = parameters["sliceSeparation"].toInt();
    int32_t noSlices = parameters["noSlices"].toInt();
    int32_t fov(parameters["fov"].toInt());
    float xAngle = parameters["xAngle"].toDouble();
    float yAngle = parameters["yAngle"].toDouble();
    float zAngle = parameters["zAngle"].toDouble();
    float xOffset = parameters["xOffset"].toDouble();
    float yOffset = parameters["yOffset"].toDouble();
    float zOffset = parameters["zOffset"].toDouble();

    size = 56 + 16;
    unsigned char *out = new unsigned char[size];
    int i = 0;
    for (const void *const v :
         {static_cast<const void *>(&observeFrequency),
          static_cast<const void *>(&noSamples),
          static_cast<const void *>(&noViews),
          static_cast<const void *>(&noViews2),
          static_cast<const void *>(&sliceThickness),
          static_cast<const void *>(&sliceSeparation),
          static_cast<const void *>(&noSlices),
          static_cast<const void *>(&fov),
          static_cast<const void *>(&xAngle),
          static_cast<const void *>(&yAngle),
          static_cast<const void *>(&zAngle),
          static_cast<const void *>(&xOffset),
          static_cast<const void *>(&yOffset),
          static_cast<const void *>(&zOffset)}) {
        memcpy(out + 16 + 4 * i++, v, 4);
    }
    setHeader(out, 5, 1, size - 16);
    return out;
}

unsigned char *encodeT2(QJsonObject &sequence, int &size, char implement=1) {
    QJsonObject parameters = sequence["parameters"].toObject();
    int observeFrequency = parameters["observeFrequency"].toInt();
    int32_t noSamples = parameters["noSamples"].toInt();
    int32_t noViews = parameters["noViews"].toInt();
    int32_t viewsPerSegment = parameters["viewsPerSegment"].toInt();
    int32_t noAverages = parameters["noAverages"].toInt();
    int32_t sliceThickness = parameters["sliceThickness"].toInt();
    int32_t fov = parameters["fov"].toInt();

    int32_t noSlices = parameters["noSlices"].toInt();
    QJsonArray slices = parameters["slices"].toArray();

    size = 16 + 32 + 24 * noSlices;
    unsigned char* out = new unsigned char[size];

    int i = 0;
    for (const void* const v : {
             static_cast<const void*>(&observeFrequency),
             static_cast<const void*>(&noSamples),
             static_cast<const void*>(&noViews),
             static_cast<const void*>(&viewsPerSegment),
             static_cast<const void*>(&noAverages),
             static_cast<const void*>(&sliceThickness),
             static_cast<const void*>(&fov),
             static_cast<const void*>(&noSlices)
         }) {
        memcpy(out + 16 + 4 * i++, v, 4);
    }

    // parameters of each slice
    for(i=0;i<slices.size();i++){
        int j = 16 + 32 + 24*i;
        QJsonObject slice = slices[i].toObject();

        float xAngle = slice["xAngle"].toDouble();
        float yAngle = slice["yAngle"].toDouble();
        float zAngle = slice["zAngle"].toDouble();
        float xOffset = slice["xOffset"].toDouble();
        float yOffset = slice["yOffset"].toDouble();
        float zOffset = slice["zOffset"].toDouble();

        // Write 6 parameters in sequence
        memcpy(out + j, &xAngle, 4);
        memcpy(out + j + 4, &yAngle, 4);
        memcpy(out + j + 8, &zAngle, 4);
        memcpy(out + j + 12, &xOffset, 4);
        memcpy(out + j + 16, &yOffset, 4);
        memcpy(out + j + 20, &zOffset, 4);
    }

    setHeader(out, 6, 1, size - 16);
    return out;
}
} // namespace

SequenceEncoder::SequenceEncoder() {}

unsigned char *SequenceEncoder::encode(QJsonObject &sequence, int &size) {
    QString seq = sequence["sequence"].toString();

    if(seq == "t1"){
        return encodeT1(sequence, size);
    }

    if(seq == "t2"){
        return encodeT2(sequence, size);
    }

    if(seq == "tune"){
        return encodeTune(sequence, size);
    }

    if(seq == "rfopt"){
        return encodeRfopt(sequence, size);
    }

    if(seq == "shim"){
        return encodeShim(sequence, size);
    }

    qDebug() << "unkown sequence";
    return nullptr;
}

unsigned char *SequenceEncoder::encodeStop(int id, int &size)
{
    size = 16;
    unsigned char *out = new unsigned char[size];
    setHeader(out, 1, 1, size-16);
    memcpy(out+4, &id, 4);
    return out;
}
