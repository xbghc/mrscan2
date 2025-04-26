#include "mrdparser.h"

#include "utils.h"
#include <QFile>
#include <QDebug>
#include <qendian.h>
#include <memory>

namespace {

struct FFTWComplexDeleter {
    void operator()(fftw_complex* ptr) const {
        if (ptr) fftw_free(ptr);
    }
};

FFTWComplexPtr createFFTWComplex(size_t size) {
    return FFTWComplexPtr(static_cast<fftw_complex*>(fftw_alloc_complex(size)));
}

std::unique_ptr<double[]> abs(fftw_complex* array, int len){
    auto res = std::make_unique<double[]>(len);
    for(int i=0;i<len;i++){
        auto re = array[i][0];
        auto im = array[i][1];
        res[i] = sqrt(re*re + im*im);
    }
    return res;
}

template <typename T>
FFTWComplexPtr parseKData(QByteArray content, int length, bool isComplex){
    auto out = createFFTWComplex(length);
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

FFTWComplexPtr exec_fft_3d(fftw_complex* in, const size_t* _n){
    int n[3];
    size_t noPixels = 1;
    for(int i=0;i<3;i++){
        n[i] = _n[i];
        noPixels *= _n[i];
    }
    if(noPixels == 0){
        LOG_ERROR("exec fft error: 0 in n");
        return nullptr;
    }

    auto out = createFFTWComplex(noPixels);
    if (!out) {
        LOG_ERROR("Failed to allocate memory for FFT output");
        return nullptr;
    }

    fftw_plan plan = fftw_plan_dft(3, n, in, out.get(), FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan) {
        LOG_ERROR("Failed to create FFT plan");
        return nullptr;
    }
    
    // 使用RAII包装器确保plan被释放
    struct FFTWPlanRAII {
        fftw_plan p;
        FFTWPlanRAII(fftw_plan plan) : p(plan) {}
        ~FFTWPlanRAII() { if (p) fftw_destroy_plan(p); }
    };
    FFTWPlanRAII planRAII(plan);
    
    fftw_execute(plan);
    return out;
}

FFTWComplexPtr exec_fft_3d(const FFTWComplexPtr& in, const size_t* _n){
    return exec_fft_3d(in.get(), _n);
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

    auto temp = createFFTWComplex(nx * ny * nz);
    if (!temp) {
        LOG_ERROR("Failed to allocate memory for fftshift");
        return;
    }

    // Iterate through each element and rearrange
    for (size_t x = 0; x < nx; ++x) {
        for (size_t y = 0; y < ny; ++y) {
            for (size_t z = 0; z < nz; ++z) {
                // Calculate new coordinates (circular shift)
                const size_t new_x = (x + sx) % nx;
                const size_t new_y = (y + sy) % ny;
                const size_t new_z = (z + sz) % nz;

                // Calculate new and old linear indices
                const size_t old_idx = x * ny * nz + y * nz + z;
                const size_t new_idx = new_x * ny * nz + new_y * nz + new_z;

                // Copy data to temporary array
                temp[new_idx][0] = data[old_idx][0];
                temp[new_idx][1] = data[old_idx][1];
            }
        }
    }

    // Copy results back to original array
    std::memcpy(data, temp.get(), sizeof(fftw_complex) * nx * ny * nz);
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
    FFTWComplexPtr dataPtr = nullptr;
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

    auto content = read(fpath);
    return parse(content);
}

QList<QImage> MrdParser::reconImages(const MrdData* mrd)
{
    if (!mrd || !mrd->kdata) {
        return QList<QImage>();
    }

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

    auto outPtr = exec_fft_3d(mrd->kdata, n);
    if (!outPtr) {
        LOG_ERROR("Failed to execute FFT");
        return QList<QImage>();
    }
    
    fftshift3d(outPtr.get(), n);

    // Take absolute values
    size_t noPixels = n[0] * n[1] * n[2];
    auto absValues = abs(outPtr.get(), noPixels);

    double max_val = 0;
    for(size_t i=0; i<noPixels; i++){
        if(absValues[i] > max_val){
            max_val = absValues[i];
        }
    }

    QList<QImage> images;
    if(mrd->slices == 1){
        // T1
        for(size_t i=0;i<noImages;i++){
            QImage img(n[2], n[0], QImage::Format_Grayscale8);
            for(size_t j=0;j<n[0];j++){
                size_t indexes[3] = {j, i, 0};
                uchar* scanLine = img.scanLine(j);
                for(size_t k=0;k<n[2];k++){
                    indexes[2] = k;
                    double val = get_3d_ele(absValues.get(), n, indexes);
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            images.push_back(img);
        }
    }else{
        // T2
        for(size_t i=0;i<noImages;i++){
            QImage img(n[2], n[1], QImage::Format_Grayscale8);
            for(size_t j=0;j<n[1];j++){
                size_t indexes[3] = {i, j, 0};
                uchar* scanLine = img.scanLine(j);
                for(size_t k=0;k<n[2];k++){
                    indexes[2] = k;
                    double val = get_3d_ele(absValues.get(), n, indexes);
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            images.push_back(img);
        }
    }

    return images;
}
