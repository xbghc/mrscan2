#ifndef EXAMRESPONSE_H
#define EXAMRESPONSE_H

#include <QImage>
#include <QVector>

class IExamResponse {
public:
    virtual ~IExamResponse() = default;
    virtual IExamResponse *clone() const = 0;

    virtual QVector<QVector<QImage>> images() const = 0;

    virtual void load(const QString &fpath) = 0;
    virtual void save(const QString &fpath) const = 0;

protected:
    IExamResponse() = default;
};

#endif // EXAMRESPONSE_H
