#include "scannerresponse.h"

#include <QDebug>
#include <QFile>

namespace{
bool checkHeader(const QByteArray& byteArray){
    return byteArray[0] == 'N' && byteArray[1] == 'M' && byteArray[2] == 'R';
}

ScannerResponse::DataType bytes2dataType(const unsigned char *bytes) {
    int value;
    memcpy(&value, bytes, 4);

    if (value == static_cast<int>(ScannerResponse::DataType::DebugImages)) {
        return ScannerResponse::DataType::DebugImages;
    }

    qDebug() << "get wrong data type: " << value;
    return ScannerResponse::DataType::Unknown;
}
}

ScannerResponse ScannerResponse::fromBytes(const QByteArray& byteArray)
{
    if(!checkHeader(byteArray)){
        qDebug() << "check header failed";
        return ScannerResponse();
    }

    auto bytes = reinterpret_cast<const unsigned char*>(byteArray.constData());
    int length = byteArray.length();

    DataType dataType = bytes2dataType(bytes+8);
    if(dataType == DataType::Unknown){
        qDebug() << "unkonwn data type";
        return ScannerResponse();
    }

    char version;
    memcpy(&version, bytes+3, 1);
    int id;
    memcpy(&id, bytes+4, 4);
    int dataSize;
    memcpy(&dataSize, bytes+12, 4);
    if(dataSize+16 != length){
        qDebug() << "<examResponse.fromBytes>wrong data size";
        return ScannerResponse();
    }

    return ScannerResponse(id, version, dataType, byteArray);
}

ScannerResponse ScannerResponse::fromFile(QString path)
{
    QFile file(path);
    if(!file.exists() || !file.open(QIODevice::ReadOnly)){
        qDebug() << "<examResponse.fromFile>can't open file";
        return ScannerResponse();
    }

    QByteArray fileContent = file.readAll();
    file.close();

    if(fileContent.isEmpty()){
        qDebug() << "<examResponse.fromFile>file is empty";
        return ScannerResponse();
    }

    return fromBytes(fileContent);
}

ScannerResponse::ScannerResponse(int _id, char _version, DataType _dataType, const QByteArray _data)
    : id(_id), version(_version), dataType(_dataType), m_data(_data)
{
}

int ScannerResponse::getId()
{
    return id;
}

const QByteArray ScannerResponse::getRawData()
{
    return m_data;
}

const QList<QImage> ScannerResponse::getImages() const
{
    if(dataType == DataType::DebugImages){
        QList<QImage> images;
        auto ptr = m_data.constData() + kHeaderSize;
        int count;
        int width;
        int height;
        memcpy(&count, ptr, 4);
        memcpy(&width, ptr+4, 4);
        memcpy(&height, ptr+8, 4);
        auto pixels = reinterpret_cast<const float *>(ptr+12);
        for(int i = 0;i<count;i++){
            QImage img(width, height, QImage::Format_Grayscale8);

            for (int h = 0; h < height; ++h) {
                uchar* scanline = img.scanLine(h);
                for (int w = 0; w < width; ++w) {
                    float value = pixels[i * width * height + h * width + w];

                    scanline[w] = static_cast<uchar>(std::clamp(value * 255.0f, 0.0f, 255.0f));
                }
            }

            images.push_back(img);
        }

        return images;
    }

    qDebug() << "get images failed";
    return QList<QImage>();
}
