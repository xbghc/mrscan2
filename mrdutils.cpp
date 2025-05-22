#include "mrdutils.h"

#include <QtEndian>
#include <vector>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

#include "utils.h"

namespace {

template <typename T>
/// 从指针中读取指定长度的整数，并转为int类型
int readInt32(const char *ptr) {
    return static_cast<int>(qFromLittleEndian<T>(ptr));
}

template <typename T>
fftw_utils::fftw_complex_ptr readKdata(const char *ptr, int nele, bool isComplex) {
    auto kdata_ptr = fftw_utils::createArray(nele);

    auto array = reinterpret_cast<const T *>(ptr);
    if (isComplex) {
        for (size_t i = 0; i < nele; i++) {
            kdata_ptr.get()[i][0] = array[2 * i];
            kdata_ptr.get()[i][1] = array[2 * i + 1];
        }
    } else {
        for (size_t i = 0; i < nele; i++) {
            kdata_ptr.get()[i][0] = array[i];
            kdata_ptr.get()[i][1] = 0;
        }
    }
    return kdata_ptr;
}

template <typename T>
std::vector<fftw_utils::fftw_complex_ptr> readKdatas(const char *ptr, int nele, bool isComplex,
                                   int totalSize) {
    auto kdataSize = nele * sizeof(T) * (isComplex ? 2 : 1);

    if (kdataSize == 0) {
        LOG_ERROR("Mrd文件数据有误: kdataSize is zero");
        return {};
    }
    if (totalSize % kdataSize != 0) {
        LOG_ERROR("Mrd文件数据有误");
        return {};
    }

    std::vector<fftw_utils::fftw_complex_ptr> kdatas_vec;
    kdatas_vec.reserve(totalSize / kdataSize);
    int noChannels = totalSize / kdataSize;
    for (int i = 0; i < noChannels; i++) {
        auto single_kdata_ptr = readKdata<T>(ptr + i * kdataSize, nele, isComplex);
        kdatas_vec.push_back(std::move(single_kdata_ptr));
    }
    return kdatas_vec;
}
} // namespace

namespace mrd_utils {

QVector<int> Mrd::shape() const {
    return {experiments, echoes, slices, views, views2, samples};
}

size_t Mrd::size() const {
    if (experiments <= 0 || echoes <= 0 || slices <= 0 || views <= 0 ||
        views2 <= 0 || samples <= 0) {
        return 0;
    }

    return static_cast<size_t>(experiments) * static_cast<size_t>(echoes) *
           static_cast<size_t>(slices) * static_cast<size_t>(views) *
           static_cast<size_t>(views2) * static_cast<size_t>(samples);
}

QVector<QImage> Mrd::images() const {
    if (!kdata.get()) {
        return {};
    }

    size_t noImages;
    std::vector<int> shape(3);
    if (slices == 1) {
        // T1
        noImages = views2;
        shape[0] = views;
        shape[1] = views2;
        shape[2] = samples;
    } else {
        // T2
        noImages = slices;
        shape[0] = slices;
        shape[1] = views;
        shape[2] = samples;
    }

    auto outPtr = fftw_utils::exec_fft_3d(kdata.get(), shape);
    if (!outPtr.get()) {
        LOG_ERROR("FFT execution failed or returned null pointer.");
        return {};
    }
    fftw_utils::fftshift3d(outPtr.get(), shape);

    // Take absolute values
    size_t noPixels = shape[0] * shape[1] * shape[2];
    auto absValues = fftw_utils::abs(outPtr.get(), noPixels);

    double max_val = 0;
    for (size_t i = 0; i < noPixels; i++) {
        if (absValues[i] > max_val) {
            max_val = absValues[i];
        }
    }

    QVector<QImage> imageList;
    if (slices == 1) {
        // T1
        for (int i = 0; i < noImages; i++) {
            QImage img(shape[2], shape[0], QImage::Format_Grayscale8);
            for (int j = 0; j < shape[0]; j++) {
                std::vector<int> indexes = {j, i, 0};
                uchar *scanLine = img.scanLine(j);
                for (int k = 0; k < shape[2]; k++) {
                    indexes[2] = k;
                    auto index = fftw_utils::getIndex(shape, indexes);
                    double val = absValues[index];
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            imageList.push_back(img);
        }
    } else {
        // T2
        for (int i = 0; i < noImages; i++) {
            QImage img(shape[2], shape[1], QImage::Format_Grayscale8);
            for (int j = 0; j < shape[1]; j++) {
                std::vector<int> indexes = {i, j, 0};
                uchar *scanLine = img.scanLine(j);
                for (int k = 0; k < shape[2]; k++) {
                    indexes[2] = k;
                    auto index = fftw_utils::getIndex(shape, indexes);
                    double val = absValues[index];
                    scanLine[k] = static_cast<uchar>(val * 255 / max_val);
                }
            }
            imageList.push_back(img);
        }
    }

    return imageList;
}

Mrd::Mrd() {}

Mrd::~Mrd() {
}

Mrd::Mrd(const Mrd &other)
    : experiments(other.experiments), echoes(other.echoes),
    slices(other.slices), views(other.views), views2(other.views2),
    samples(other.samples) {
    if (!other.kdata.get()) {
        return;
    }

    auto num_elements = other.size();
    if (num_elements == 0) return;

    kdata = fftw_utils::createArray(num_elements);
    memcpy(kdata.get(), other.kdata.get(), num_elements * sizeof(fftw_complex));
}

Mrd &Mrd::operator=(const Mrd &other) {
    if (this == &other) {
        return *this;
    }

    Mrd temp(other);
    swap(temp);
    return *this;
}

Mrd::Mrd(Mrd &&other) noexcept
    : kdata(nullptr), experiments(0), echoes(0), slices(0), views(0), views2(0),
    samples(0) {
    swap(other);
}

Mrd &Mrd::operator=(Mrd &&other) noexcept {
    swap(other);

    return *this;
}

void Mrd::swap(Mrd &other) noexcept {
    using std::swap;

    swap(kdata, other.kdata);
    swap(experiments, other.experiments);
    swap(echoes, other.echoes);
    swap(slices, other.slices);
    swap(views, other.views);
    swap(views2, other.views2);
    swap(samples, other.samples);
}

/**
 * @brief Mrd::fromBytes
 * @param bytes
 * @return
 * @ref https://github.com/hongmingjian/mrscan/blob/master/smisscanner.py#L34
 * function: SmisScanner.parseMrd
 */
QVector<Mrd> Mrd::fromBytes(const QByteArray &bytes) {
    if (bytes.size() < 512) {
        LOG_ERROR(QString("传入了长度为%1的mrd文件，mrd文件的长度至少为512")
                      .arg(bytes.size()));
        return {};
    }

    auto rawData = bytes.constData();

    // 解析头部
    auto samples = readInt32<qint32>(rawData + 0);
    auto views = readInt32<qint32>(rawData + 4);
    auto views2 = readInt32<qint32>(rawData + 8);
    auto slices = readInt32<qint32>(rawData + 12);
    // 16-18 Unspecified
    auto datatype = readInt32<qint16>(rawData + 18);
    // 20-152 Unspecified
    auto echoes = readInt32<qint32>(rawData + 152);
    auto experiments = readInt32<qint32>(rawData + 156);

    int nele = experiments * echoes * slices * views * views2 * samples;

    // 截取数据部分
    const int kdataOffset = 512;
    auto posPPR = bytes.lastIndexOf('\x00');
    if (posPPR < 0) {
        LOG_ERROR("错误的mrd文件");
        return {};
    }
    int totalSize = posPPR + 1 - kdataOffset - 120;
    if (totalSize < 0) {
        LOG_ERROR("Invalid totalSize calculated for Mrd data.");
        return {};
    }

    // 解析数据部分
    std::vector<fftw_utils::fftw_complex_ptr> kdatas_ptr_vec;
    bool isComplex = datatype & 0x10;
    switch (datatype & 0xf) {
    case 0:
        kdatas_ptr_vec =
            readKdatas<quint8>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 1:
        kdatas_ptr_vec =
            readKdatas<qint8>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 2:
        kdatas_ptr_vec =
            readKdatas<quint16>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 3:
        kdatas_ptr_vec =
            readKdatas<qint16>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 4:
        kdatas_ptr_vec =
            readKdatas<quint32>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 5:
        kdatas_ptr_vec =
            readKdatas<qint32>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 6:
        kdatas_ptr_vec =
            readKdatas<float>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 7:
        kdatas_ptr_vec =
            readKdatas<double>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    default:
        LOG_ERROR(QString("Unknown datatype: %1").arg(datatype));
        return {};
    }

    QVector<Mrd> results;
    for (auto &k_ptr : kdatas_ptr_vec) {
        Mrd m;
        m.samples = samples;
        m.views = views;
        m.views2 = views2;
        m.slices = slices;
        m.echoes = echoes;
        m.experiments = experiments;
        m.kdata = std::move(k_ptr);
        results.push_back(std::move(m));
    }

    return results;
}

void swap(Mrd &lhs, Mrd &rhs) noexcept { lhs.swap(rhs); }

QStringList getAllChannelsFile(const QString& path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        LOG_WARNING(QString("File path does not exist: %1").arg(path));
        return {};
    }

    QDir dir = fileInfo.absoluteDir();
    QString fileName = fileInfo.fileName();

    static QRegularExpression namePattern("^(.*)#(\\d+)\\.(\\w+)$");
    QRegularExpressionMatch match = namePattern.match(fileName);
    if (!match.hasMatch()) {
        LOG_WARNING("Invalid filename format, expected format: prefix#number.suffix");
        return {};
    }

    QString prefix = QRegularExpression::escape(match.captured(1));
    QString suffix = QRegularExpression::escape(match.captured(3));

    // Build new regex to match related files
    QString pattern = QString("^%1#\\d+\\.%2$").arg(prefix, suffix);
    QRegularExpression regex(pattern);

    QStringList result;
    const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
    for (const QString& file : files) {
        if (regex.match(file).hasMatch()) {
            result.append(dir.filePath(file));
        }
    }

    return result;
}

} // namespace mrd_utils
