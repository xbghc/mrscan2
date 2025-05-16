#include "mrdresponse.h"

#include "mrdutils.h"

MrdResponse::MrdResponse() {}

MrdResponse::MrdResponse(QByteArray data) : m_data(data) {}

IExamResponse *MrdResponse::clone() const { return new MrdResponse(m_data); }

QVector<QVector<QImage>> MrdResponse::images() const {
    QVector<QVector<QImage>> imageList;

    for(const auto& mrd:mrd_utils::Mrd::fromBytes(m_data)){
        imageList.push_back(mrd.images());
    }

    return imageList;
}

QByteArray MrdResponse::bytes() const
{
    return m_data;
}

