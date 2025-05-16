#include "mrdutils.h"

#include <QtEndian>

#include "utils.h"

namespace {

template <typename T>
/// 从指针中读取指定长度的整数，并转为int类型
int readInt32(const char *ptr) {
    return static_cast<int>(qFromLittleEndian<T>(ptr));
}

template <typename T>
fftw_complex *readKdata(const char *ptr, int nele, bool isComplex) {
    auto kdata = fftw_utils::createArray(nele);

    auto array = reinterpret_cast<const T *>(ptr);
    if (isComplex) {
        for (size_t i = 0; i < nele; i++) {
            kdata[i][0] = array[2 * i];
            kdata[i][1] = array[2 * i + 1];
        }
    } else {
        for (size_t i = 0; i < nele; i++) {
            kdata[i][0] = array[i];
            kdata[i][1] = 0;
        }
    }
    return kdata;
}

template <typename T>
QVector<fftw_complex *> readKdatas(const char *ptr, int nele, bool isComplex,
                                   int totalSize) {
    auto kdataSize = nele * sizeof(T) * (isComplex ? 2 : 1);

    if (totalSize % kdataSize != 0) {
        LOG_ERROR("Mrd文件数据有误");
        return {};
    }

    QVector<fftw_complex *> kdatas;
    int noChannels = totalSize / kdataSize;
    for (int i = 0; i < noChannels; i++) {
        auto kdata = readKdata<T>(ptr + i * kdataSize, nele, isComplex);
        kdatas.push_back(kdata);
    }
    return kdatas;
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

Mrd::Mrd() {}

Mrd::~Mrd() {
    if (kdata) {
        fftw_free(kdata);
        kdata = nullptr;
    }
}

Mrd::Mrd(const Mrd &other)
    : kdata(nullptr), experiments(other.experiments), echoes(other.echoes),
    slices(other.slices), views(other.views), views2(other.views2),
    samples(other.samples) {
    if (!other.kdata) {
        return;
    }

    auto num_elements = other.size();
    kdata = static_cast<fftw_complex *>(fftw_alloc_complex(num_elements));
    if (!kdata) {
        throw std::bad_alloc();
    }
    memcpy(kdata, other.kdata, num_elements * sizeof(fftw_complex));
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

    // 解析数据部分
    QVector<fftw_complex *> kdatas;
    bool isComplex = datatype & 0x10;
    switch (datatype & 0xf) {
    case 0:
        kdatas =
            readKdatas<quint8>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 1:
        kdatas =
            readKdatas<qint8>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 2:
        // Case 2 and 3 are the same
    case 3:
        kdatas =
            readKdatas<qint16>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 4:
        kdatas =
            readKdatas<qint32>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 5:
        kdatas =
            readKdatas<float>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    case 6:
        kdatas =
            readKdatas<double>(rawData + kdataOffset, nele, isComplex, totalSize);
        break;
    default:
        LOG_ERROR("Unknown data type in the MRD file!");
        return {};
    }

    QVector<Mrd> mrds;
    for (int i = 0; i < kdatas.size(); i++) {
        Mrd mrd;
        mrd.kdata = kdatas[i];
        mrd.samples = samples;
        mrd.views = views;
        mrd.views2 = views2;
        mrd.slices = slices;
        mrd.echoes = echoes;
        mrd.experiments = experiments;
        mrds.push_back(mrd);
    }

    return mrds;
}

void swap(Mrd &lhs, Mrd &rhs) noexcept { lhs.swap(rhs); }

} // namespace mrd_utils
