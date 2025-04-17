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
{
    ui->setupUi(this);

    // TODO 不知道为什么默认高度会出错，需要手动调整
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->resize(QSize(this->width(), this->height()));

    // start 尝试修复默认高度问题
    // 使用最小大小提示确保窗口有合理大小
    this->setMinimumSize(1024, 768);
    
    // 确保合理的大小策略以适应内容
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 确保布局正确计算
    this->adjustSize();
    
    LOG_INFO("主窗口初始化");
    // end 尝试修复默认高度问题

    // 如果没有提供扫描仪适配器，创建一个默认的
    if (adapter == nullptr) {
        adapter = new ScannerAdapter;
    }
    
    adapter->open();

    // 连接扫描信号到ExamTab
    connect(adapter, &IScannerAdapter::scanStarted, ui->examTab, &ExamTab::onScanStarted);
    connect(adapter, &IScannerAdapter::scanEnded, this, &MainWindow::handleScanComplete);
    
    // 从ExamTab接收扫描操作信号
    connect(ui->examTab, &ExamTab::onStopButtonClicked, this, &MainWindow::handleScanStop);
    connect(ui->examTab, &ExamTab::onStartButtonClicked, this, &MainWindow::handleScanStart);
    connect(ui->examTab, &ExamTab::fileSaved, this, &MainWindow::handleExamHistorySaved);

    // 当显示图像时
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
    LOG_INFO("主窗口销毁");
    delete ui;
    
    if (adapter) {
        adapter->close();
        if (ownAdapter) {
            delete adapter;
        }
    }
}

void MainWindow::handleScanStart(QJsonObject sequence)
{
    LOG_INFO("开始扫描");
    adapter->scan(sequence);
}

void MainWindow::handleScanStop(int id)
{
    LOG_INFO(QString("停止扫描 ID: %1").arg(id));
    int stoppedId = adapter->stop(id);
    
    if (stoppedId != id) {
        LOG_ERROR(QString("停止扫描的ID不匹配, 预期: %1, 实际: %2").arg(id).arg(stoppedId));
    }
    
    // 更新UI
    ui->examTab->updateScanButtonState(false);
    ui->examTab->enablePatientSelection(true);
}

void MainWindow::handleScanComplete(QByteArray response)
{
    LOG_INFO("扫描完成，保存数据");
    
    // 获取患者ID和当前扫描数据
    int patientId = ui->examTab->getCurrentPatientId();
    
    // 创建历史记录
    QJsonObject request = ui->examTab->getCurrentExam();
    ExamHistory history(request, response);
    history.setPatient(patientId);
    
    // 保存历史记录
    if (history.save()) {
        LOG_INFO("历史记录保存成功");
    } else {
        LOG_ERROR("历史记录保存失败");
    }
    
    // 通知UI已保存
    ui->examTab->onScanEnd(response);
    ui->examTab->fileSaved(history);
}

void MainWindow::handleExamHistorySaved(ExamHistory history)
{
    // 加载扫描结果图像
    ui->imagesWidget->loadMrdFiles(history.responsePath());
    
    // 更新历史列表
    ui->historyTab->loadHistoryList();
    
    LOG_INFO("历史记录已更新");
}
