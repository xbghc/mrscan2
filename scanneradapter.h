#ifndef SCANNERADAPTER_H
#define SCANNERADAPTER_H

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QThread>
#include <QObject>
#include <memory>

#include "virtualscanner.h"
#include "exam.h"
#include "examresponse.h"

// Define scanner adapter interface
class IScanner : public QObject
{
    Q_OBJECT
public:
    IScanner(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IScanner() = default;

    /**
     * @brief 打开扫描仪
     * @retval ==0 成功
     * @retval !=0 失败
     */
    virtual int open() = 0;
    /**
     * @brief 关闭扫描仪
     * @retval ==0 成功
     * @retval !=0 失败
     */
    virtual int close() = 0;

    virtual bool isConnected() const = 0;

    virtual void scan(const ExamRequest& request) = 0;
    virtual QString stop(QString id) = 0;

signals:
    void started(QString id);
    void completed(IExamResponse* response);
    void stoped(QString id);
};

// Concrete implementation class
class ScannerAdapter: public IScanner
{
    Q_OBJECT
public:
    ScannerAdapter(QObject* parent=nullptr);
    ~ScannerAdapter();

    bool isConnected() const override { return m_isConnected; }
    int open() override;
    void scan(const ExamRequest& request) override;
    QString stop(QString id) override;
    int close() override;

private:
    bool m_isConnected{false};

    QString newId();
};

#endif // SCANNERADAPTER_H
