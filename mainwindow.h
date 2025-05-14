#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <memory>
#include "scanneradapter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, IScannerAdapter* scannerAdapter = nullptr);
    ~MainWindow();

private slots:
    void handleScanStop(QString id);
    void handleScanComplete(QByteArray response);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<IScannerAdapter> adapter;
    std::unique_ptr<QThread> workerThread;

};
#endif // MAINWINDOW_H
