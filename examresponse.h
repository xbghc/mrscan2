#ifndef EXAMRESPONSE_H
#define EXAMRESPONSE_H

#include <QImage>
#include <QVector>

class IExamResponse
{
public:
    virtual ~IExamResponse()=default;
    virtual IExamResponse* clone() const=0;

    virtual QVector<QVector<QImage>> images() const=0;

    virtual void load(const QString& fpath) = 0;
    virtual void save(const QString& fpath)const = 0;
protected:
    IExamResponse()=default;
};

class MrdResponse: public IExamResponse{
public:
    MrdResponse();
    MrdResponse(QByteArray data);
    ~MrdResponse()=default;
    IExamResponse* clone()const override;

    QVector<QVector<QImage>> images() const override;
    void load(const QString& fpath) override;
    void save(const QString& fpath)const override;
private:
    QByteArray m_data;
};

#endif // EXAMRESPONSE_H
