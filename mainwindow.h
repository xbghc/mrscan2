#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scanneradapter.h"
#include "examhistory.h"

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
    void handleScanStart(QJsonObject sequence);
    void handleScanStop(int id);
    void handleScanComplete(QByteArray response);
    void handleExamHistorySaved(ExamHistory history);

private:
    Ui::MainWindow *ui;
    IScannerAdapter *adapter;
    bool ownAdapter;  // 标记是否需要在析构时删除适配器
};
#endif // MAINWINDOW_H
