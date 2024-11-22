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

    // display images
    connect(ui->examTab, &ExamTab::displayExam, this, [this](ExamHistory history){
        for(auto& image:history.images()){
            ui->imagesWidget->addImage(image);
        }
    });
    connect(ui->historyTab, &HistoryTab::currentHistoryChanged, this, [this](ExamHistory history){
        for(auto& image:history.images()){
            ui->imagesWidget->addImage(image);
        }
    });

    // scanned
    connect(ui->examTab, &ExamTab::scanned, this, [this](){
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
}
