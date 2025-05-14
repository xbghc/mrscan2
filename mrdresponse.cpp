#include "examresponse.h"
#include "mrdparser.h"

MrdResponse::MrdResponse(){

}

MrdResponse::MrdResponse(QByteArray data)
    :m_data(data)
{

}

IExamResponse *MrdResponse::clone() const
{
    return new MrdResponse(m_data);
}

QVector<QVector<QImage>> MrdResponse::images() const{
    auto mrd = MrdParser::parse(m_data);
    return MrdParser::reconImages(mrd.get());
}

void MrdResponse::load(const QString &fpath)
{
    m_data = FileUtils::read(fpath);
}

void MrdResponse::save(const QString &fpath) const
{
    FileUtils::save(fpath, m_data);
}
