#include "scanneradapter.h"
#include "fakescanner.h"
#include "sequenceencoder.h"

#include <random>

namespace {

double generate_random_double() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(0.0, 1.0);

    return dis(gen);
}

QImage generate_random_qimage() {
    const static int width = 256;
    const static int height = 256;

    QImage image(width, height, QImage::Format_Grayscale8);

    for(int y = 0; y < height; ++y) {
        uchar* line = image.scanLine(y);
        for(int x = 0; x < width; ++x) {
            line[x] = static_cast<uchar>(generate_random_double() * 255);
        }
    }

    return image;
}

QList<QImage> generate_random_qimages(int num){
    QList<QImage> out;
    for(int i=0;i<num;i++){
        out.append(generate_random_qimage());
    }
    return out;
}

QList<QJsonObject> split_slices_parameters(QJsonObject &sequence){
    // todo
    int noSlices = sequence["noSlice"].toInt();
    QList<QJsonObject> out;
    for(int i=0;i<noSlices;i++){
        out.append(QJsonObject());
    }
    return out;
}

} // namespace

ScannerAdapter::ScannerAdapter() {}

int ScannerAdapter::open() { return FakeScanner::open(); }

void ScannerAdapter::scan(
    QJsonObject &sequence,
    std::function<void(QList<QImage>, QList<QJsonObject>)> callback)
{
    // todo
    int noSlices = sequence["noSlices"].toInt();


    callback(generate_random_qimages(noSlices), split_slices_parameters(sequence));
}

int ScannerAdapter::close() { return FakeScanner::close(); }
