#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "preferencesdialog.h"
#include "tuningcentralfrequency.h"
#include "tuningradiofrequencypower.h"
#include "tuningshimming.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent, IScannerAdapter* scannerAdapter)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workerThread(new QThread)
{
    ui->setupUi(this);

    // TODO Don't know why the default height is wrong, need to adjust manually
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->resize(QSize(this->width(), this->height()));

    // start： Attempt to fix default height issue
    // Use minimum size hint to ensure window has reasonable size
    this->setMinimumSize(1024, 768);
    
    // Ensure reasonable size policy to fit content
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Ensure layout is calculated correctly
    this->adjustSize();
    
    LOG_INFO("Main window initialized");
    // end： Attempt to fix default height issue

    // If no scanner adapter is provided, create a default one
    if (scannerAdapter == nullptr) {
        adapter = std::make_unique<ScannerAdapter>();
    } else {
        adapter.reset(scannerAdapter);
    }
    
    adapter->moveToThread(workerThread.get());
    workerThread->start();
    adapter->open();

    // Connect scan signals to ExamTab
    connect(adapter.get(), &IScannerAdapter::scanStarted, ui->examTab, &ExamTab::onScanStarted);
    connect(adapter.get(), &IScannerAdapter::scanEnded, this, &MainWindow::handleScanComplete);
    
    // Receive scan operation signals from ExamTab
    connect(ui->examTab, &ExamTab::stopButtonClicked, this, &MainWindow::handleScanStop);
    connect(ui->examTab, &ExamTab::startButtonClicked, adapter.get(), &IScannerAdapter::scan);

    // preference
    connect(ui->actionPreferences, &QAction::triggered, this, [this]() {
        PreferencesDialog dialog(this);
        dialog.setModal(true);
        dialog.exec();
    });

    // tunings
    connect(ui->actionCentral_Frequency, &QAction::triggered, this, [this]() {
        TuningCentralFrequency dialog(this);
        dialog.setModal(true);
        dialog.exec();
    });
    connect(ui->actionRF_Power, &QAction::triggered, this, [this]() {
        TuningRadioFrequencyPower dialog(this);
        dialog.setModal(true);
        dialog.exec();
    });
    connect(ui->actionShimming, &QAction::triggered, this, [this]() {
        TuningShimming dialog(this);
        dialog.setModal(true);
        dialog.exec();
    });
}

MainWindow::~MainWindow()
{
    LOG_INFO("Main window destroyed");
    
    if (adapter) {
        adapter->close();
    }
    
    workerThread->quit();
    workerThread->wait();
}

void MainWindow::handleScanStop(QString id)
{
    LOG_INFO(QString("Stop scanning ID: %1").arg(id));
    auto stoppedId = adapter->stop(id);
    
    if (stoppedId != id) {
        LOG_ERROR(QString("Stopped scan ID does not match, expected: %1, actual: %2").arg(id).arg(stoppedId));
    }
    
    // Update UI
    ui->examTab->updateScanButtonState(false);
    ui->examTab->enablePatientSelection(true);
}

void MainWindow::handleScanComplete(QByteArray response)
{

    LOG_INFO("Scan completed, saving data");

    const auto& exam = ui->examTab->onResponseReceived(response);

    ui->imagesWidget->setData(exam);
    ui->historyTab->loadHistoryList();

    LOG_INFO("History record updated");
}
