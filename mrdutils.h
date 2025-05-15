#ifndef MRDUTILS_H
#define MRDUTILS_H

#include <QVector>
#include <fftw3.h>

namespace mrd_utils {
struct Mrd {
    fftw_complex *kdata=nullptr;
    int experiments = 0;
    int echoes = 0;
    int slices = 0;
    int views = 0;
    int views2 = 0;
    int samples = 0;

    QVector<int> shape() const;
    size_t size() const;

    Mrd();
    ~Mrd();
    Mrd(const Mrd &other);
    Mrd &operator=(const Mrd &other);
    Mrd(Mrd &&other) noexcept;
    Mrd &operator=(Mrd &&other) noexcept;
    void swap(Mrd &other) noexcept;
};

void swap(Mrd& lhs, Mrd& rhs) noexcept;

} // namespace mrd_utils

#endif // MRDUTILS_H
