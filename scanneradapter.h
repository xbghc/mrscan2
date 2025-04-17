#ifndef SCANNERADAPTER_H
#define SCANNERADAPTER_H

#include <QByteArray>
#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QThread>
#include <QObject>

#include "virtualscanner.h"

// 定义扫描仪适配器接口
class IScannerAdapter : public QObject
{
    Q_OBJECT
public:
    IScannerAdapter(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IScannerAdapter() = default;
    virtual int open() = 0;
    virtual void scan(QJsonObject &sequence) = 0;
    virtual int stop(int id) = 0;
    virtual int close() = 0;
    virtual bool isConnected() const = 0;

signals:
    void scanStarted(int id);
    void scanEnded(QByteArray response);
    void stoped(int id);
};

// 具体实现类
class ScannerAdapter: public IScannerAdapter
{
    Q_OBJECT
public:
    ScannerAdapter(QObject* parent=nullptr);
    ~ScannerAdapter();
    
    int open() override;
    void scan(QJsonObject &sequence) override;
    int stop(int id) override;
    int close() override;
    bool isConnected() const override { return m_isConnected; }

private:
    VirtualScanner scanner;
    bool m_isConnected{false};
};

#endif // SCANNERADAPTER_H
