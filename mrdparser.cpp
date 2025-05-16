#include "mrdparser.h"

#include "utils.h"
#include <QDebug>
#include <QFile>
#include <qendian.h>

QVector<QVector<QImage>> MrdParser::reconImages(const mrd_utils::Mrd *mrd) {
    QVector<QVector<QImage>> ans;
    if (!mrd || !mrd->kdata) {
        return ans;
    }

    size_t noImages;
    std::vector<int> shape(3);
    if (mrd->slices == 1) {
        // T1
        noImages = mrd->views2;
        shape[0] = mrd->views;
        shape[1] = mrd->views2;
        shape[2] = mrd->samples;
    } else {
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
    for (size_t i = 0; i < noPixels; i++) {
        if (absValues[i] > max_val) {
            max_val = absValues[i];
        }
    }

    QVector<QImage> images;
    if (mrd->slices == 1) {
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
            images.push_back(img);
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
            images.push_back(img);
        }
    }

    ans.append(images);
    return ans;
}
