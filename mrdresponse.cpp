#include "examresponse.h"
#include "mrdparser.h"
#include "mrdutils.h"

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

    // auto mrd = mrd_utils::Mrd::fromBytes(m_data).at(0);
    // return MrdParser::reconImages(&mrd);
}

void MrdResponse::load(const QString &fpath)
{
    m_data = file_utils::read(fpath);
}

void MrdResponse::save(const QString &fpath) const
{
    file_utils::save(fpath, m_data);
}
