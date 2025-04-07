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

    // TODO 不知道为什么默认高度会出错，需要手动调整
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->resize(QSize(this->width(), this->height()));

    adapter = new ScannerAdapter;
    adapter->open();

    connect(adapter, &ScannerAdapter::scanStarted, ui->examTab, &ExamTab::onScanStarted);
    connect(adapter, &ScannerAdapter::scanEnded, ui->examTab, &ExamTab::onScanEnd);
    connect(ui->examTab, &ExamTab::onStopButtonClicked, adapter, &ScannerAdapter::stop);
    connect(ui->examTab, &ExamTab::onStartButtonClicked, adapter, &ScannerAdapter::scan);

    // display images
    connect(ui->examTab, &ExamTab::fileSaved, this, [this](ExamHistory history){
        ui->imagesWidget->loadMrdFiles(history.responsePath());
    });

    connect(ui->historyTab, &HistoryTab::currentIndexChanged, this, [this](ExamHistory history){
        ui->imagesWidget->loadMrdFiles(history.responsePath());
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
