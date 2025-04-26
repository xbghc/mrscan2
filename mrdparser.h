#ifndef MRDPARSER_H
#define MRDPARSER_H

#include <QImage>
#include <QList>
#include <QString>
#include <fftw3.h>
#include <memory>

// 自定义FFTW资源的删除器
struct FFTWComplexDeleter {
    void operator()(fftw_complex* ptr) const {
        if (ptr) fftw_free(ptr);
    }
};

using FFTWComplexPtr = std::unique_ptr<fftw_complex[], FFTWComplexDeleter>;

// kdata.shape = experiments,echoes,slices,views,views2,samples
struct MrdData{
    FFTWComplexPtr kdata;
    size_t samples;
    size_t views;
    size_t views2;
    size_t slices;
    size_t echoes;
    size_t experiments;

    MrdData() {}
    
    // 支持移动构造和赋值
    MrdData(MrdData&& other) noexcept = default;
    MrdData& operator=(MrdData&& other) noexcept = default;
    
    // 禁止拷贝
    MrdData(const MrdData&) = delete;
    MrdData& operator=(const MrdData&) = delete;
};

class MrdParser
{
private:
    MrdParser() = delete;
    MrdParser(const MrdParser&) = delete;
    MrdParser& operator=(const MrdParser&) = delete;

public:
    static std::unique_ptr<MrdData> parse(const QByteArray& content);
    static std::unique_ptr<MrdData> parseFile(QString fpath);
    static QList<QImage> reconImages(const MrdData* mrd);
};

#endif // MRDPARSER_H
