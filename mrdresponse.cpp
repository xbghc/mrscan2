#include "mrdresponse.h"

#include "mrdutils.h"

MrdResponse::MrdResponse() {}

MrdResponse::MrdResponse(QByteArray data) : m_data(data) {}

IExamResponse *MrdResponse::clone() const { return new MrdResponse(m_data); }

QVector<QVector<QImage>> MrdResponse::images() const {
    auto mrd = mrd_utils::Mrd::fromBytes(m_data).at(0);
    return {mrd.images()};
}

QByteArray MrdResponse::bytes() const
{
    return m_data;
}

