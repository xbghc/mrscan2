#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "preferencesdialog.h"
#include "tuningcentralfrequency.h"
#include "tuningradiofrequencypower.h"
#include "tuningshimming.h"
#include "utils.h"
#include "exammodel.h"

MainWindow::MainWindow(QWidget *parent, IScannerAdapter* scannerAdapter)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , adapter(scannerAdapter)
    , ownAdapter(scannerAdapter == nullptr)
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
    if (adapter == nullptr) {
        adapter = new ScannerAdapter;
    }
    adapter->moveToThread(workerThread);
    workerThread->start();
    adapter->open();

    // Connect scan signals to ExamTab
    connect(adapter, &IScannerAdapter::scanStarted, ui->examTab, &ExamTab::onScanStarted);
    connect(adapter, &IScannerAdapter::scanEnded, this, &MainWindow::handleScanComplete);
    
    // Receive scan operation signals from ExamTab
    connect(ui->examTab, &ExamTab::onStopButtonClicked, this, &MainWindow::handleScanStop);
    connect(ui->examTab, &ExamTab::onStartButtonClicked, adapter, &IScannerAdapter::scan);
    connect(ui->examTab, &ExamTab::fileSaved, this, &MainWindow::handleExamHistorySaved);

    // When displaying images
    connect(ui->historyTab, &HistoryTab::currentIndexChanged, this, [this](ExamHistory history){
        ui->imagesWidget->loadMrdFiles(history.responsePath());
    });

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
    delete ui;
    workerThread->quit();
    workerThread->wait();
    delete workerThread;
    
    if (adapter) {
        adapter->close();
        if (ownAdapter) {
            delete adapter;
        }
    }
}

void MainWindow::handleScanStop(int id)
{
    LOG_INFO(QString("Stop scanning ID: %1").arg(id));
    int stoppedId = adapter->stop(id);
    
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
    
    // Get patient ID and current scan data
    int patientId = ui->examTab->getCurrentPatientId();
    
    // Create history record
    QJsonObject request = ui->examTab->getCurrentExam();
    ExamHistory history(request, response);
    history.setPatient(patientId);
    
    // Save history record
    if (history.save()) {
        LOG_INFO("History record saved successfully");
    } else {
        LOG_ERROR("Failed to save history record");
    }
    
    // Notify UI that scan is saved
    ui->examTab->onScanEnd(response);
    ui->examTab->fileSaved(history);
}

void MainWindow::handleExamHistorySaved(ExamHistory history)
{
    // Load scan result images
    ui->imagesWidget->loadMrdFiles(history.responsePath());
    
    // Update history list
    ui->historyTab->loadHistoryList();
    
    LOG_INFO("History record updated");
}
