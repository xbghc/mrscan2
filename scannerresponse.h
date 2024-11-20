#ifndef SCANNERRESPONSE_H
#define SCANNERRESPONSE_H

#include <QList>
#include <QImage>

class ScannerResponse
{
public:
    static const size_t kHeaderSize = 16;
    static const size_t kDataSizeOffset = 12;

    enum class DataType: int32_t{
        Unknown = 0,
        DebugImages = 1,
    }; // Remember to update byte2dataType() when adding new enum values

    static ScannerResponse fromBytes(const QByteArray& byteArray);
    static ScannerResponse fromFile(QString path);

    QList<QImage> getImages();

    ScannerResponse() = default;
    ScannerResponse(int _id, char _version, DataType _dataType, const QByteArray _data);

    int getId();
    const QByteArray getRawData();

private:
    int id;
    char version;
    DataType dataType;
    QByteArray m_data;
};

#endif // SCANNERRESPONSE_H
