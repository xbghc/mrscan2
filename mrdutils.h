#ifndef MRDUTILS_H
#define MRDUTILS_H

#include <QImage>
#include <QVector>
#include "utils.h"

namespace mrd_utils {
struct Mrd {
    fftw_utils::fftw_complex_ptr kdata = nullptr;
    int experiments = 0;
    int echoes = 0;
    int slices = 0;
    int views = 0;
    int views2 = 0;
    int samples = 0;

    QVector<int> shape() const;
    size_t size() const;
    QVector<QImage> images()const;

    Mrd();
    ~Mrd();
    Mrd(const Mrd &other);
    Mrd &operator=(const Mrd &other);
    Mrd(Mrd &&other) noexcept;
    Mrd &operator=(Mrd &&other) noexcept;
    void swap(Mrd &other) noexcept;

    static QVector<Mrd> fromBytes(const QByteArray &bytes);
};

void swap(Mrd &lhs, Mrd &rhs) noexcept;

/**
 * @brief 获取同文件夹下所有通道的文件
 * @param path 某一通道的文件路径
 * @return 所有通道的文件路径
 */
QStringList getAllChannelsFile(const QString& path);

} // namespace mrd_utils

#endif // MRDUTILS_H
