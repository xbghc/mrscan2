#ifndef MRDRESPONSE_H
#define MRDRESPONSE_H

#include "examresponse.h"

class MrdResponse : public IExamResponse {
public:
    MrdResponse();
    MrdResponse(QByteArray data);
    IExamResponse *clone() const override;

    QVector<QVector<QImage>> images() const override;

    QByteArray bytes() const override;

private:
    QByteArray m_data;
};

#endif // MRDRESPONSE_H
