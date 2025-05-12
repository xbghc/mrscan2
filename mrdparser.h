#ifndef MRDPARSER_H
#define MRDPARSER_H

#include "utils.h"

#include <QImage>
#include <QList>
#include <QString>
#include <fftw3.h>

/*@brief kdata.shape = experiments,echoes,slices,views,views2,samples
 *@detail 尽量不将fftw_complex对外暴露，处理好垃圾回收
*/
struct MrdData{
    fftw_complex* kdata;
    size_t samples;
    size_t views;
    size_t views2;
    size_t slices;
    size_t echoes;
    size_t experiments;

    MrdData() {}
    ~MrdData() {if(kdata)fftw_free(kdata);}

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
    static QVector<QVector<QImage>> reconImages(const MrdData* mrd);
};

#endif // MRDPARSER_H
