#ifndef MRDPARSER_H
#define MRDPARSER_H

#include "utils.h"

#include <QImage>
#include <QList>
#include <QString>
#include <fftw3.h>

#include "mrdutils.h"

class MrdParser
{
private:
    MrdParser() = delete;
    MrdParser(const MrdParser&) = delete;
    MrdParser& operator=(const MrdParser&) = delete;

public:
    static std::unique_ptr<mrd_utils::Mrd> parse(const QByteArray& content);
    static std::unique_ptr<mrd_utils::Mrd> parseFile(QString fpath);
    static QVector<QVector<QImage>> reconImages(const mrd_utils::Mrd* mrd);
};

#endif // MRDPARSER_H
