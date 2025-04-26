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

// Define scanner adapter interface
class IScannerAdapter : public QObject
{
    Q_OBJECT
public:
    IScannerAdapter(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IScannerAdapter() = default;

    virtual bool isConnected() const = 0;
    virtual int open() = 0;
    virtual void scan(QJsonObject sequence) = 0;
    virtual int stop(int id) = 0;
    virtual int close() = 0;

signals:
    void scanStarted(int id);
    void scanEnded(QByteArray response);
    void stoped(int id);
};

// Concrete implementation class
class ScannerAdapter: public IScannerAdapter
{
    Q_OBJECT
public:
    ScannerAdapter(QObject* parent=nullptr);
    ~ScannerAdapter();

    bool isConnected() const override { return m_isConnected; }
    int open() override;
    void scan(QJsonObject sequence) override;
    int stop(int id) override;
    int close() override;

private:
    std::unique_ptr<VirtualScanner> scanner;
    bool m_isConnected{false};
};

#endif // SCANNERADAPTER_H
