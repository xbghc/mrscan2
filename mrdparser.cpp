#include "mrdparser.h"

#include "utils.h"
#include <QFile>
#include <QDebug>
#include <qendian.h>
#include <memory>

namespace {

template <typename T>
fftw_complex* parseKData(QByteArray content, int length, bool isComplex){
    auto out = fftw_utils::createArray(length);
    if (!out) {
        LOG_WARNING("Failed to allocate memory for kdata");
        return nullptr;
    }

    auto array = reinterpret_cast<const T*>(content.constData() + 512);
    if(isComplex){
        for(size_t i=0;i<length;i++){
            out[i][0] = array[2*i];
            out[i][1] = array[2*i+1];
        }
    }else{
        for(size_t i=0;i<length;i++){
            out[i][0] = array[i];
            out[i][1] = 0;
        }
    }
    return out;
}

} // namespace

std::unique_ptr<MrdData> MrdParser::parse(const QByteArray &content)
{
    // header
    int samples = qFromLittleEndian<qint32>(content.constData());
    int views = qFromLittleEndian<qint32>(content.constData() + 4);
    int views2 = qFromLittleEndian<qint32>(content.constData() + 8);
    int slices = qFromLittleEndian<qint32>(content.constData() + 12);
    // 16-18 Unspecified
    int datatype = qFromLittleEndian<qint16>(content.constData() + 18);
    // 20-152 Unspecified
    int echoes = qFromLittleEndian<qint32>(content.constData() + 152);
    int experiments = qFromLittleEndian<qint32>(content.constData() + 156);

    int nele = experiments * echoes * slices * views * views2 * samples;

    // switch datatype
    fftw_complex* dataPtr = nullptr;
    bool isComplex = datatype & 0x10;

    switch(datatype & 0xf){
    case 0:
        dataPtr = parseKData<quint8>(content, nele, isComplex);
        break;
    case 1:
        dataPtr = parseKData<qint8>(content, nele, isComplex);
        break;
    case 2:
        // Case 2 and 3 are the same
    case 3:
        dataPtr = parseKData<qint16>(content, nele, isComplex);
        break;
    case 4:
        dataPtr = parseKData<qint32>(content, nele, isComplex);
        break;
    case 5:
        dataPtr = parseKData<float>(content, nele, isComplex);
        break;
    case 6:
        dataPtr = parseKData<double>(content, nele, isComplex);
        break;
    default:
        LOG_ERROR("Unknown data type in the MRD file!");
        return nullptr;
    }

    if (!dataPtr) {
        return nullptr;
    }

    auto mrdData = std::make_unique<MrdData>();
    mrdData->kdata = std::move(dataPtr);
    mrdData->echoes = echoes;
    mrdData->experiments = experiments;
    mrdData->samples = samples;
    mrdData->slices = slices;
    mrdData->views = views;
    mrdData->views2 = views2;

    return mrdData;
}

std::unique_ptr<MrdData> MrdParser::parseFile(QString fpath)
{
    // ref:
    //   url: https://github.com/hongmingjian/mrscan/blob/master/smisscanner.py
    //   function: SmisScanner.parseMrd

    auto content = file_utils::read(fpath);
    return parse(content);
}


QVector<QVector<QImage>> MrdParser::reconImages(const MrdData* mrd)
{
    QVector<QVector<QImage>> ans;
    if (!mrd || !mrd->kdata) {
        return ans;
    }

    size_t noImages;
    std::vector<int> shape(3);
    if(mrd->slices == 1){
        // T1
        noImages = mrd->views2;
        shape[0] = mrd->views;
        shape[1] = mrd->views2;
        shape[2] = mrd->samples;
    }else{
        // T2
        noImages = mrd->slices;
        shape[0] = mrd->slices;
        shape[1] = mrd->views;
        shape[2] = mrd->samples;
    }

    auto outPtr = fftw_utils::exec_fft_3d(mrd->kdata, shape);
    if (!outPtr) {
        LOG_ERROR("Failed to execute FFT");
        return ans;
    }
    
    fftw_utils::fftshift3d(outPtr, shape);

    // Take absolute values
    size_t noPixels = shape[0] * shape[1] * shape[2];
    auto absValues = fftw_utils::abs(outPtr, noPixels);

    double max_val = 0;
    for(size_t i=0; i<noPixels; i++){
        if(absValues[i] > max_val){
            max_val = absValues[i];
        }
    }

    QVector<QImage> images;
    if(mrd->slices == 1){
        // T1
        for(int i=0;i<noImages;i++){
            QImage img(shape[2], shape[0], QImage::Format_Grayscale8);
            for(int j=0;j<shape[0];j++){
                std::vector<int> indexes = {j, i, 0};
                uchar* scanLine = img.scanLine(j);
                for(int k=0;k<shape[2];k++){
                    indexes[2] = k;
                    auto index = fftw_utils::getIndex(shape, indexes);
                    double val = absValues[index];
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            images.push_back(img);
        }
    }else{
        // T2
        for(int i=0;i<noImages;i++){
            QImage img(shape[2], shape[1], QImage::Format_Grayscale8);
            for(int j=0;j<shape[1];j++){
                std::vector<int> indexes = {i, j, 0};
                uchar* scanLine = img.scanLine(j);
                for(int k=0;k<shape[2];k++){
                    indexes[2] = k;
                    auto index = fftw_utils::getIndex(shape, indexes);
                    double val = absValues[index];
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            images.push_back(img);
        }
    }


    ans.append(images);
    return ans;
}
