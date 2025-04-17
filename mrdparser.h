#ifndef MRDPARSER_H
#define MRDPARSER_H

#include <QImage>
#include <QList>
#include <QString>
#include <fftw3.h>

// kdata.shape = experiments,echoes,slices,views,views2,samples
struct MrdData{
    fftw_complex* kdata;
    size_t samples;
    size_t views;
    size_t views2;
    size_t slices;
    size_t echoes;
    size_t experiments;

    MrdData(){}
    ~MrdData(){
        fftw_free(kdata);
    }
};

class MrdParser
{
private:
    MrdParser() = delete;
    MrdParser(const MrdParser&) = delete;
    MrdParser& operator=(const MrdParser&) = delete;

public:
    static MrdData* parse(const QByteArray& content);
    static MrdData* parseFile(QString fpath);
    static QList<QImage> reconImages(MrdData* mrd);
};

#endif // MRDPARSER_H
