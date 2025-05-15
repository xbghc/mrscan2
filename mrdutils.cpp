#include "mrdutils.h"

namespace mrd_utils{

QVector<int> Mrd::shape() const
{
    return {experiments, echoes, slices, views, views2, samples};
}

size_t Mrd::size() const
{
    if (experiments <= 0 || echoes <= 0 || slices <= 0 || views <= 0 || views2 <= 0 || samples <= 0) {
        return 0;
    }

    return static_cast<size_t>(experiments) *
           static_cast<size_t>(echoes) *
           static_cast<size_t>(slices) *
           static_cast<size_t>(views) *
           static_cast<size_t>(views2) *
           static_cast<size_t>(samples);
}

Mrd::Mrd() {}

Mrd::~Mrd() {
    if (kdata) {
        fftw_free(kdata);
        kdata = nullptr;
    }
}

Mrd::Mrd(const Mrd& other)
    : kdata(nullptr),
    experiments(other.experiments),
    echoes(other.echoes),
    slices(other.slices),
    views(other.views),
    views2(other.views2),
    samples(other.samples)
{
    if(!other.kdata){
        return;
    }

    auto num_elements = other.size();
    kdata = static_cast<fftw_complex*>(fftw_alloc_complex(num_elements));
    if (!kdata) {
        throw std::bad_alloc();
    }
    memcpy(kdata, other.kdata, num_elements * sizeof(fftw_complex));
}

Mrd& Mrd::operator=(const Mrd& other)
{
    if(this == &other){
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

Mrd& Mrd::operator=(Mrd&& other) noexcept
{
    swap(other);

    return *this;
}

void Mrd::swap(Mrd &other) noexcept
{
    using std::swap;

    swap(kdata, other.kdata);
    swap(experiments, other.experiments);
    swap(echoes, other.echoes);
    swap(slices, other.slices);
    swap(views, other.views);
    swap(views2, other.views2);
    swap(samples, other.samples);
}

void swap(Mrd &lhs, Mrd &rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace mrd_utils
