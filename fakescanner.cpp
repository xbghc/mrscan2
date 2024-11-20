#include "fakescanner.h"

#include <cstring>
#include <QDebug>
#include <QJsonObject>
#include <random>

namespace{
QJsonObject decodeHeader(const unsigned char *buf){
    if(buf[0] != 'N' && buf[1] != 'M' && buf[2] != 'R'){
        qDebug() << "wrong header";
        return QJsonObject();
    }

    if(buf[3] != 1){
        qDebug() << "unsupported protocol version";
        return QJsonObject();
    }

    QJsonObject header;

    int id;
    memcpy(&id, buf + 4, 4);
    header["id"] = id;

    char seqType;
    memcpy(&seqType, buf+10, 1);
    header["type"] = static_cast<int>(seqType);

    char implement;
    memcpy(&implement, buf+11, 1);
    header["implement"] = static_cast<int>(implement);

    int dataSize;
    memcpy(&dataSize, buf+12, 4);
    header["dataSize"] = dataSize;

    return header;
}

const unsigned char* mockImagesResult(int id, int count, int width, int height){
    unsigned char* out = new unsigned char[16 + 12 + count * width * height * 4];

    out[0] = 'N';
    out[1] = 'M';
    out[2] = 'R';
    out[3] = 1;

    memcpy(out+4, &id, 4);

    int dataType = 1;
    memcpy(out+8, &dataType, 4);

    int dataSize = 12 + count * width * height * 4;
    memcpy(out+12, &dataSize, 4);

    memcpy(out+16, &count, 4);
    memcpy(out+20, &width, 4);
    memcpy(out+24, &height, 4);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);

    for(int i=0;i<count*width*height;i++){
        float pixel = dis(gen);
        memcpy(out + 16+12+4*i, &pixel, 4);
    }
    return out;
}

}

int FakeScanner::stop(int id, int implement){
    if(scanningId == -1){
        qDebug() << "nothing is scanning";
        return -1;
    }

    if(scanningId != id){
        qDebug() << "wrong! Id tried to stop is not running";
        return -1;
    }

    if(implement != 1){
        qDebug() << "unsupport implement stop";
        return -1;
    }

    qDebug() << "stop success! id: " << id;
    scanningId = -1;
    return id;
}

int FakeScanner::tune(int id, int implement, int dataSize, const unsigned char* data){
    // todo
    if(implement != 1){
        qDebug() << "unsupport implement stop";
        return -1;
    }

    qDebug() << "tune success! id: " << id;
    scanningId = id;
    return id;
}

int FakeScanner::rfopt(int id, int implement, int dataSize, const unsigned char* data){
    // todo

    scanningId = id;
    return id;
}

int FakeScanner::shimming(int id, int implement, int dataSize, const unsigned char* data){
    // todo

    scanningId = id;
    return id;
}

int FakeScanner::t1(int id, int implement, int dataSize, const unsigned char* data){
    if(!content.empty()){
        qDebug() << "can't strat new exam when data has not been completely read";
        return -1;
    }

    if(implement != 1){
        qDebug() << "unsupport implement stop";
        return -1;
    }

    if(dataSize != 56){
        qDebug() << "dataSize should be 56";
        return -1;
    }

    int noSamples;
    memcpy(&noSamples, data+4, 4);
    int noViews;
    memcpy(&noViews, data+8, 4);
    int noViews2;
    memcpy(&noViews2, data+12, 4);

    auto result = mockImagesResult(id, noViews2, noSamples, noViews);
    content.assign(result, result+16+12+4*noViews2*noSamples*noViews);

    scanningId = id;
    return id;
}

int FakeScanner::t2(int id, int implement, int dataSize, const unsigned char* data){
    // todo
    if(!content.empty()){
        qDebug() << "can't strat new exam when data has not been completely read";
        return -1;
    }

    if(implement != 1){
        qDebug() << "unsupport implement stop";
        return -1;
    }

    int noSamples;
    memcpy(&noSamples, data+4, 4);
    int noViews;
    memcpy(&noViews, data+8, 4);
    int noSlices;
    memcpy(&noSlices, data+28, 4);

    auto result = mockImagesResult(id, noSlices, noSamples, noViews);
    content.assign(result, result+16+12+4*noSlices*noSamples*noViews);

    scanningId = id;
    return id;
}


FakeScanner::FakeScanner() {
    scanningId = -1;
}

int FakeScanner::open()
{
    return 0;
}

int FakeScanner::close()
{
    return 0;
}

int FakeScanner::write(const unsigned char *buf, int len)
{
    QJsonObject header = decodeHeader(buf);

    int seqType = header["type"].toInt();
    int dataSize = header["dataSize"].toInt();
    if(dataSize + 16 != len){
        qDebug() << "length doesn't match";
        return -1;
    }

    int implement = header["implement"].toInt();
    int id = header["id"].toInt();

    switch(seqType){
    case 1: // stop
        stop(id, implement);
        return len;
        break;
    case 2: // tune
        tune(id, implement, dataSize, buf+16);
        return len;
        break;
    case 3: // rfopt
        rfopt(id, implement, dataSize, buf + 16);
        return len;
        break;
    case 4: // shimming
        shimming(id, implement, dataSize, buf + 16);
        return len;
        break;
    case 5: // t1
        t1(id, implement, dataSize, buf + 16);
        return len;
        break;
    case 6: // t2
        t2(id, implement, dataSize, buf + 16);
        return len;
        break;
    default:
        break;
    }

    qDebug() << "fake device get unknown sequence type";
    return -1;
}

int FakeScanner::read(unsigned char *buf, int len)
{
    if(buf == nullptr || len <= 0){
        qDebug() << "buf is nullptr or len <= 0";
        return 0;
    }

    if(content.empty() || position >= content.size()){
        return 0;
    }

    const size_t remainingBytes = content.size() - position;
    const size_t bytesToRead = std::min(static_cast<size_t>(len), remainingBytes);

    memcpy(buf, content.data() + position, bytesToRead);
    position += bytesToRead;

    if(position >= content.size()){
        cleanup();
    }

    return static_cast<int>(bytesToRead);
}
