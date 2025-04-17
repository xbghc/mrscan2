#include "mrdparser.h"

#include "utils.h"
#include <QFile>
#include <QDebug>
#include <qendian.h>

namespace {

double* abs(fftw_complex* array, int len){
    double* res = new double[len];
    for(int i=0;i<len;i++){
        auto re = array[i][0];
        auto im = array[i][1];
        res[i] = sqrt(re*re + im*im);
    }
    return res;
}

template <typename T>
fftw_complex* parseKData(QByteArray content, int length, bool isComplex){
    auto out = (fftw_complex*) fftw_alloc_complex(length);

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

fftw_complex* exec_fft_3d(fftw_complex* in, const size_t* _n){
    int n[3];
    size_t noPixels = 1;
    for(int i=0;i<3;i++){
        n[i] = _n[i];
        noPixels *= _n[i];
    }
    if(noPixels == 0){
        qDebug() << "exec fft error: 0 in n";
        return nullptr;
    }

    auto out = (fftw_complex*) fftw_alloc_complex(noPixels);

    fftw_plan plan = fftw_plan_dft(3, n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    return out;
}

template<typename T>
T get_3d_ele(T* array, const size_t* array_shape, const size_t* indexes){
    size_t offset = indexes[0] * array_shape[1] * array_shape[2] +
                    indexes[1] * array_shape[2] + indexes[2];
    return array[offset];
}

template<typename T>
void set_3d_ele(T* array, const size_t* array_shape, const size_t* indexes, const T& val){
    size_t offset = indexes[0] * array_shape[1] * array_shape[2] +
                    indexes[1] * array_shape[2] + indexes[2];
    array[offset] = val;
}

void fftshift3d(fftw_complex* data, const size_t* shape) {
    const size_t nx = shape[0];
    const size_t ny = shape[1];
    const size_t nz = shape[2];

    const size_t sx = nx / 2;
    const size_t sy = ny / 2;
    const size_t sz = nz / 2;

    // 分配临时数组
    fftw_complex* temp = fftw_alloc_complex(nx * ny * nz);

    // 遍历每个元素并重新排列
    for (size_t x = 0; x < nx; ++x) {
        for (size_t y = 0; y < ny; ++y) {
            for (size_t z = 0; z < nz; ++z) {
                // 计算新坐标（循环移位）
                const size_t new_x = (x + sx) % nx;
                const size_t new_y = (y + sy) % ny;
                const size_t new_z = (z + sz) % nz;

                // 计算新旧线性索引
                const size_t old_idx = x * ny * nz + y * nz + z;
                const size_t new_idx = new_x * ny * nz + new_y * nz + new_z;

                // 复制数据到临时数组
                temp[new_idx][0] = data[old_idx][0];
                temp[new_idx][1] = data[old_idx][1];
            }
        }
    }

    // 将结果复制回原数组
    std::memcpy(data, temp, sizeof(fftw_complex) * nx * ny * nz);

    // 释放临时内存
    fftw_free(temp);
}
} // namespace

MrdData *MrdParser::parse(const QByteArray &content)
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
    fftw_complex* data;
    bool isComplex = datatype & 0x10;

    switch(datatype & 0xf){
    case 0:
        data = parseKData<quint8>(content, nele, isComplex);
        break;
    case 1:
        data = parseKData<qint8>(content, nele, isComplex);
        break;
    case 2:
        // 2和3一样
    case 3:
        data = parseKData<qint16>(content, nele, isComplex);
        break;
    case 4:
        data = parseKData<qint32>(content, nele, isComplex);
        break;
    case 5:
        data = parseKData<float>(content, nele, isComplex);
        break;
    case 6:
        data = parseKData<double>(content, nele, isComplex);
        break;
    default:
        qDebug() << "Unknown data type in the MRD file!";
        return nullptr;
    }

    auto mrdData = new MrdData;
    mrdData->kdata = data;
    mrdData->echoes = echoes;
    mrdData->experiments = experiments;
    mrdData->samples = samples;
    mrdData->slices = slices;
    mrdData->views = views;
    mrdData->views2 = views2;

    return mrdData;
}

MrdData* MrdParser::parseFile(QString fpath)
{
    // ref:
    //   url: https://github.com/hongmingjian/mrscan/blob/master/smisscanner.py
    //   function: SmisScanner.parseMrd

    auto content = read(fpath);
    return parse(content);
}

QList<QImage> MrdParser::reconImages(MrdData* mrd)
{
    size_t noImages;
    size_t n[3];
    if(mrd->slices == 1){
        // T1
        noImages = mrd->views2;
        n[0] = mrd->views;
        n[1] = mrd->views2;
        n[2] = mrd->samples;
    }else{
        // T2
        noImages = mrd->slices;
        n[0] = mrd->slices;
        n[1] = mrd->views;
        n[2] = mrd->samples;
    }
    auto out = exec_fft_3d(mrd->kdata, n);
    fftshift3d(out, n);

    // 取绝对值
    size_t noPixels = n[0] * n[1] * n[2];
    auto images_data = abs(out, noPixels);
    fftw_free(out);

    // 归一化并返回图片
    // 整个数组归一化
    double max = 0;
    for(size_t i=0;i<noImages;i++){
        for(size_t y=0;y<n[1];y++){
            for(size_t x=0;x<n[2];x++){
                size_t pos[3] = {i, y, x};
                auto pixel = get_3d_ele(images_data, n, pos);
                if(pixel>max){
                    max = pixel;
                }
            }
        }
    }

    QList<QImage> images;
    images.reserve(noImages);
    if(mrd->slices == 1){
        // T1
        for(size_t i=0;i<noImages;i++){
            // 单张图片归一化
            // double max = 0;
            // for(size_t y=0;y<n[0];y++){
            //     for(size_t x=0;x<n[2];x++){
            //         size_t pos[3] = {y, i, x};
            //         auto pixel = get_3d_ele(images_data, n, pos);
            //         if(max < pixel){
            //             max = pixel;
            //         }
            //     }
            // }

            QImage img(n[2], n[0], QImage::Format_Grayscale8);
            for(size_t y=0;y<n[0];y++){
                auto scanLine = img.scanLine(y);
                for(size_t x=0;x<n[2];x++){
                    size_t pos[3] = {y, i, x};
                    auto pixel = get_3d_ele(images_data, n, pos);
                    scanLine[x] = static_cast<uchar>(pixel / max * 255.0);
                }
            }
            images.push_back(img);
        }
    }else{
        // T2
        for(size_t i=0;i<noImages;i++){
            // 单张图片归一化
            // double max = 0;
            // for(size_t y=0;y<n[1];y++){
            //     for(size_t x=0;x<n[2];x++){
            //         size_t pos[3] = {i, y, x};
            //         auto pixel = get_3d_ele(images_data, n, pos);
            //         if(max < pixel){
            //             max = pixel;
            //         }
            //     }
            // }

            QImage img(n[2], n[1], QImage::Format_Grayscale8);
            for(size_t y=0;y<n[1];y++){
                auto scanLine = img.scanLine(y);
                for(size_t x=0;x<n[2];x++){
                    size_t pos[3] = {i, y, x};
                    auto pixel = get_3d_ele(images_data, n, pos);
                    scanLine[x] = static_cast<uchar>(pixel / max * 255.0);
                }
            }
            images.push_back(img);
        }
    }

    delete[] images_data;
    return images;
}
