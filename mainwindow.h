#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
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
    void handleScanStop(int id);
    void handleScanComplete(QByteArray response);
    void handleExamHistorySaved(ExamHistory history);

private:
    Ui::MainWindow *ui;
    IScannerAdapter *adapter;
    bool ownAdapter;  // Flag whether the adapter needs to be deleted in the destructor
    QThread *workerThread;

};
#endif // MAINWINDOW_H
