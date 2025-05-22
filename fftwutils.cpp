#include "utils.h"

namespace fftw_utils{
fftw_complex_ptr createArray(size_t size){
    auto ptr = static_cast<fftw_complex*>(fftw_alloc_complex(size));
    if(!ptr){
        auto msg = QString("尝试创建大小为%i的fftw_complex数组时失败").arg(size);
        LOG_ERROR(msg);
        throw std::runtime_error(msg.toStdString());
    }
    return fftw_complex_ptr(ptr);
}

std::vector<double> abs(fftw_complex* array, size_t len){
    auto magnitude = std::vector<double>(len);
    for(int i=0;i<len;i++){
        auto real = array[i][0];
        auto imag = array[i][1];
        magnitude[i] = sqrt(real*real + imag*imag);
    }
    return magnitude;
}

fftw_complex_ptr exec_fft_3d(fftw_complex* in, std::vector<int> n){
    size_t noPixels = 1;
    for(int i=0;i<3;i++){
        noPixels *= n[i];
    }
    if(noPixels == 0){
        LOG_ERROR("exec fft error: 0 in n");
        return {};
    }

    auto out = fftw_utils::createArray(noPixels);
    fftw_plan plan = fftw_plan_dft(3, n.data(), in, out.get(), FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan) {
        LOG_ERROR("Failed to create FFT plan");
        fftw_free(out.release()); // 如果plan创建失败，需要释放out的内存，因为此时out还未被传回，智能指针不会自动释放
        return {};
    }

    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return out;
}

void fftshift3d(fftw_complex* data, std::vector<int> shape) {
    const size_t nx = shape[0];
    const size_t ny = shape[1];
    const size_t nz = shape[2];

    const size_t sx = nx / 2;
    const size_t sy = ny / 2;
    const size_t sz = nz / 2;

    auto temp = fftw_utils::createArray(nx * ny * nz);

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

    std::memcpy(data, temp.get(), sizeof(fftw_complex) * nx * ny * nz);
}

int getIndex(std::vector<int> shape, std::vector<int> indices){
    if(shape.size() != indices.size()){
        throw std::runtime_error("Shape and indices size mismatch.");
    }

    int dim = shape.size();
    int index = 0;
    for(int i=0;i<dim;i++){
        index *= shape[i];
        index += indices[i];
    }
    return index;
}

} // namespace FFTW
