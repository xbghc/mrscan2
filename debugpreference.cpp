#include "debugpreference.h"
#include "ui_debugpreference.h"
#include "debugconfig.h"
#include "utils.h"

#include <QFileDialog>
#include <QStandardPaths>

DebugPreference::DebugPreference(QWidget *parent)
    : IPreferenceWidget(parent)
    , ui(new Ui::DebugPreference)
{
    ui->setupUi(this);
    
    // 连接信号槽
    connect(ui->browseButton, &QPushButton::clicked, this, &DebugPreference::onBrowseButtonClicked);
    connect(ui->browseLogPathButton, &QPushButton::clicked, this, &DebugPreference::onBrowseLogPathButtonClicked);
    connect(ui->mockFilePathEdit, &QLineEdit::textChanged, this, &DebugPreference::onMockFilePathChanged);
    connect(ui->logFilePathEdit, &QLineEdit::textChanged, this, &DebugPreference::onLogFilePathChanged);
    connect(ui->scanTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DebugPreference::onScanTimeChanged);
    connect(ui->enableDelayCheckBox, &QCheckBox::toggled, this, &DebugPreference::onEnableDelayChanged);
    connect(ui->logLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DebugPreference::onLogLevelChanged);
    connect(ui->logToFileCheckBox, &QCheckBox::toggled, this, &DebugPreference::onLogToFileChanged);
}

DebugPreference::~DebugPreference()
{
}

void DebugPreference::load()
{
    // 加载Scanner设置
    ui->mockFilePathEdit->setText(config::Debug::mockFilePath());
    ui->scanTimeSpinBox->setValue(config::Debug::scanTime());
    ui->enableDelayCheckBox->setChecked(config::Debug::enableScanDelay());
    
    // 加载日志设置
    ui->logLevelComboBox->setCurrentIndex(config::Debug::logLevel());
    ui->logToFileCheckBox->setChecked(config::Debug::logToFile());
    ui->logFilePathEdit->setText(config::Debug::logFilePath());
    
    LOG_INFO("Debug preferences loaded");
}

void DebugPreference::save()
{
    // 保存Scanner设置
    config::Debug::setMockFilePath(ui->mockFilePathEdit->text());
    config::Debug::setScanTime(ui->scanTimeSpinBox->value());
    config::Debug::setEnableScanDelay(ui->enableDelayCheckBox->isChecked());
    
    // 保存日志设置
    config::Debug::setLogLevel(ui->logLevelComboBox->currentIndex());
    config::Debug::setLogToFile(ui->logToFileCheckBox->isChecked());
    config::Debug::setLogFilePath(ui->logFilePathEdit->text());
    
    LOG_INFO("Debug preferences saved");
}

void DebugPreference::onBrowseButtonClicked()
{
    QString currentPath = ui->mockFilePathEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择Mock数据文件"),
        currentPath,
        tr("MRD文件 (*.mrd);;所有文件 (*.*)")
    );
    
    if (!fileName.isEmpty()) {
        ui->mockFilePathEdit->setText(fileName);
    }
}

void DebugPreference::onBrowseLogPathButtonClicked()
{
    QString currentPath = ui->logFilePathEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("选择日志文件保存路径"),
        currentPath,
        tr("日志文件 (*.log);;文本文件 (*.txt);;所有文件 (*.*)")
    );
    
    if (!fileName.isEmpty()) {
        ui->logFilePathEdit->setText(fileName);
    }
}

void DebugPreference::onMockFilePathChanged()
{
    // 实时保存文件路径变更
    config::Debug::setMockFilePath(ui->mockFilePathEdit->text());
}

void DebugPreference::onScanTimeChanged()
{
    // 实时保存扫描时间变更
    config::Debug::setScanTime(ui->scanTimeSpinBox->value());
}

void DebugPreference::onEnableDelayChanged()
{
    // 实时保存延时设置变更
    config::Debug::setEnableScanDelay(ui->enableDelayCheckBox->isChecked());
}

void DebugPreference::onLogLevelChanged()
{
    // 实时保存日志级别变更
    config::Debug::setLogLevel(ui->logLevelComboBox->currentIndex());
    
    // 应用日志级别设置
    LogLevel level = static_cast<LogLevel>(ui->logLevelComboBox->currentIndex());
    Logger::setMinLogLevel(level);
}

void DebugPreference::onLogToFileChanged()
{
    // 实时保存文件日志设置变更
    config::Debug::setLogToFile(ui->logToFileCheckBox->isChecked());
    
    // 应用文件日志设置
    Logger::setLogToFile(ui->logToFileCheckBox->isChecked(), config::Debug::logFilePath());
}

void DebugPreference::onLogFilePathChanged()
{
    // 实时保存日志文件路径变更
    config::Debug::setLogFilePath(ui->logFilePathEdit->text());
    
    // 应用日志文件路径设置
    Logger::setLogToFile(config::Debug::logToFile(), ui->logFilePathEdit->text());
}