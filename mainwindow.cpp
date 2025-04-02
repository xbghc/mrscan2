#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "preferencesdialog.h"
#include "tuningcentralfrequency.h"
#include "tuningradiofrequencypower.h"
#include "tuningshimming.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    adapter = new ScannerAdapter;
    adapter->open();

    connect(adapter, &ScannerAdapter::onScanStarted, ui->examTab, &ExamTab::onScanStarted);
    connect(adapter, &ScannerAdapter::scanned, ui->examTab, &ExamTab::onScanEnd);
    connect(ui->examTab, &ExamTab::onStopButtonClicked, adapter, &ScannerAdapter::stop);
    connect(ui->examTab, &ExamTab::onStartButtonClicked, adapter, &ScannerAdapter::scan);

    // display images
    connect(ui->examTab, &ExamTab::displayExam, this, [this](ExamHistory history){

    });
    connect(ui->historyTab, &HistoryTab::currentHistoryChanged, this, [this](ExamHistory history){

    });

    // scanned
    connect(ui->examTab, &ExamTab::fileSaved, this, [this](){
        ui->historyTab->loadHistoryList();
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
    delete ui;
    adapter->close();
    delete adapter;
}
