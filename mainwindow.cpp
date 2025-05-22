#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "preferencesdialog.h"
#include "store.h"
#include "tuningcentralfrequency.h"
#include "tuningradiofrequencypower.h"
#include "tuningshimming.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent, IScanner* scanner)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workerThread(new QThread)
{
    ui->setupUi(this);
    
    LOG_INFO("Main window initialized");

    // If no scanner is provided, create a default one
    if (scanner == nullptr) {
        m_scanner = std::make_unique<VScanner>();
    } else {
        m_scanner.reset(scanner);
    }
    
    m_scanner->moveToThread(workerThread.get());
    workerThread->start();
    m_scanner->open();

    // Connect scan signals to ExamTab
    connect(m_scanner.get(), &IScanner::started, ui->examTab, &ExamTab::onScanStarted);
    connect(m_scanner.get(), &IScanner::completed, this, &MainWindow::onScanCompleted);
    connect(m_scanner.get(), &IScanner::stoped, ui->examTab, &ExamTab::onScanStoped);
    
    // Receive scan operation signals from ExamTab
    connect(ui->examTab, &ExamTab::stopButtonClicked, this, &MainWindow::handleScanStop);
    connect(ui->examTab, &ExamTab::startButtonClicked, m_scanner.get(), &IScanner::scan);

    // 查看历史记录
    connect(ui->historyTab, &HistoryTab::currentItemChanged,
            this, [this](const Exam& exam){
        ui->imagesWidget->setData(exam);
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
    
    if (m_scanner) {
        m_scanner->close();
    }
    
    workerThread->quit();
    workerThread->wait();
}

void MainWindow::handleScanStop(QString id)
{
    LOG_INFO(QString("Requesting to stop scan, ID: %1").arg(id));
    m_scanner->stop(id);
}

void MainWindow::onScanCompleted(IExamResponse* response)
{
    LOG_INFO("Scan completed, saving data");

    const auto& exam = ui->examTab->setResponse(response);

    store::saveExam(exam);

    ui->imagesWidget->setData(exam);
    ui->historyTab->loadHistoryList(); /// @todo 这个不太好

    LOG_INFO("History record updated");
}
