#ifndef MRDRESPONSE_H
#define MRDRESPONSE_H

#include "examresponse.h"

class MrdResponse : public IExamResponse {
public:
    MrdResponse();
    MrdResponse(QByteArray data);
    ~MrdResponse() = default;
    IExamResponse *clone() const override;

    QVector<QVector<QImage>> images() const override;
    void load(const QString &fpath) override;
    void save(const QString &fpath) const override;

private:
    QByteArray m_data;
};

#endif // MRDRESPONSE_H
