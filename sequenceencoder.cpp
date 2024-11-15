#include "sequenceencoder.h"

#include <QJsonArray>

namespace {
static_assert(sizeof(int32_t) == 4, "uint32_t must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");

const unsigned char *encodeT1(QJsonObject &sequence, int &size) {
    float observeFrequency = sequence["observeFrequency"].toDouble();
    int32_t noSamples = sequence["noSamples"].toInt();
    int32_t noViews = sequence["noViews"].toInt();
    int32_t noViews2 = sequence["noViews2"].toInt();
    int32_t sliceThickness = sequence["sliceThickness"].toInt();
    int32_t sliceSeparation = sequence["sliceSeparation"].toInt();
    int32_t noSlices = sequence["noSlices"].toInt();
    int32_t fov(sequence["fov"].toInt());
    float xAngle = sequence["xAngle"].toDouble();
    float yAngle = sequence["yAngle"].toDouble();
    float zAngle = sequence["zAngle"].toDouble();
    float xOffset = sequence["xOffset"].toDouble();
    float yOffset = sequence["yOffset"].toDouble();
    float zOffset = sequence["zOffset"].toDouble();

    unsigned char *out = new unsigned char[56];
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
        memcpy(out + 4 * i++, v, 4);
    }
    size = 56;
    return out;
}

const unsigned char *encodeT2(QJsonObject &sequence, int &size) {
    float observeFrequency = sequence["observeFrequency"].toDouble();
    int32_t noSamples = sequence["noSamples"].toInt();
    int32_t noViews = sequence["noViews"].toInt();
    int32_t viewsPerSegment = sequence["viewsPerSegment"].toInt();
    int32_t noAverages = sequence["noAverages"].toInt();
    int32_t sliceThickness = sequence["sliceThickness"].toInt();
    int32_t fov = sequence["fov"].toInt();
    int32_t noSlices = sequence["noSlices"].toInt();
    QJsonArray slices = sequence["slices"].toArray();

    // 计算总大小：基础参数(32字节) + 每个切片的参数(24字节 * 切片数)
    size = 32 + 24 * noSlices;
    unsigned char* out = new unsigned char[size];

    // 写入基础参数
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
        memcpy(out + 4 * i++, v, 4);
    }

    // 获取并写入每个切片的参数
    for(i=0;i<slices.size();i++){
        int j = 32 + 24*i;
        QJsonObject slice = slices[i].toObject();

        float xAngle = slice["xAngle"].toDouble();
        float yAngle = slice["yAngle"].toDouble();
        float zAngle = slice["zAngle"].toDouble();
        float xOffset = slice["xOffset"].toDouble();
        float yOffset = slice["yOffset"].toDouble();
        float zOffset = slice["zOffset"].toDouble();

        // 按顺序写入6个参数
        memcpy(out + j, &xAngle, 4);
        memcpy(out + j + 4, &yAngle, 4);
        memcpy(out + j + 8, &zAngle, 4);
        memcpy(out + j + 12, &xOffset, 4);
        memcpy(out + j + 16, &yOffset, 4);
        memcpy(out + j + 20, &zOffset, 4);
    }

    return out;
}
} // namespace

SequenceEncoder::SequenceEncoder() {}

const unsigned char *SequenceEncoder::encode(QJsonObject &sequence, int &size) {
    QString seq = sequence["sequence"].toString();
    if(seq == "t1"){
        return encodeT1(sequence, size);
    }else if(seq == "t2"){
        return encodeT2(sequence, size);
    }

    qDebug() << "unkown sequence";
    return nullptr;
}
