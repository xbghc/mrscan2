#include "../resultwidget.h"
#include "ui_resultwidget.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsView>
#include <QMessageBox>
#include <QApplication>
#include <QRegularExpression>

#include "mrdparser.h"

namespace {
// 从给定路径获取所有通道文件
QStringList getAllChannelsFile(const QString& path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        LOG_WARNING(QString("文件路径不存在: %1").arg(path));
        return {};
    }

    QDir dir = fileInfo.absoluteDir();
    QString fileName = fileInfo.fileName();

    static QRegularExpression namePattern("^(.*)#(\\d+)\\.(\\w+)$");
    QRegularExpressionMatch match = namePattern.match(fileName);
    if (!match.hasMatch()) {
        LOG_WARNING("文件名格式无效，预期格式: 前缀#数字.后缀");
        return {};
    }

    QString prefix = QRegularExpression::escape(match.captured(1));
    QString suffix = QRegularExpression::escape(match.captured(3));

    // 构建新的正则表达式来匹配相关文件
    QString pattern = QString("^%1#\\d+\\.%2$").arg(prefix, suffix);
    QRegularExpression regex(pattern);
    
    QStringList result;
    const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
    for (const QString& file : files) {
        if (regex.match(file).hasMatch()) {
            result.append(dir.filePath(file));
        }
    }
    
    return result;
}

} // namespace

ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResultWidget)
{
    ui->setupUi(this);
    initializeUI();
    setupConnections();
}

ResultWidget::~ResultWidget()
{
    clear();
    delete ui;
}

void ResultWidget::initializeUI()
{
    // 设置UI组件的初始配置
    ui->ChannelBox->setMinimumWidth(60);
    ui->ImageBox->setMinimumWidth(80);
    
    // 初始化图像显示区域
    ui->contentWidget->init(
        ui->rowSpin->value(),
        ui->columnSpin->value(),
        ui->widthSpin->value(),
        ui->heightSpin->value()
    );
}

void ResultWidget::setupConnections()
{
    // 连接选择框变更信号
    connect(ui->ChannelBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    
    // 连接布局控制变更信号
    connect(ui->rowSpin, &QSpinBox::valueChanged, this, &ResultWidget::setRowNum);
    connect(ui->columnSpin, &QSpinBox::valueChanged, this, &ResultWidget::setColNum);
    connect(ui->widthSpin, &QSpinBox::valueChanged, this, &ResultWidget::setWidth);
    connect(ui->heightSpin, &QSpinBox::valueChanged, this, &ResultWidget::setHeight);
}

int ResultWidget::loadMrdFiles(QString fpath)
{    
    // 清除旧数据
    clear();
    
    // 获取所有通道文件
    QStringList files = getAllChannelsFile(fpath);
    if (files.isEmpty()) {
        QMessageBox::warning(this, tr("加载错误"), tr("未找到有效的通道文件: %1").arg(fpath));
        return 0;
    }
    
    // 显示加载进度
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // 加载所有通道文件
    for (const auto& file : files) {
        try {
            // 解析文件
            MrdData* content = MrdParser::parseFile(file);
            if (content == nullptr) {
                LOG_WARNING(QString("文件解析失败: %1").arg(file));
                continue;
            }
            
            // 重建图像
            auto images = MrdParser::reconImages(content);
            if (images.isEmpty()) {
                LOG_WARNING(QString("图像重建失败: %1").arg(file));
                continue;
            }
            
            m_channels.push_back(images);
            
            // 更新通道列表，提取通道编号
            QString label = extractChannelLabel(file);
            ui->ChannelBox->addItem(label, m_channels.size() - 1);
        } catch (const std::exception& e) {
            LOG_ERROR(QString("处理文件时发生错误: %1, 错误: %2").arg(file).arg(e.what()));
        }
    }
    
    // 恢复光标
    QApplication::restoreOverrideCursor();
    
    // 如果没有成功加载任何通道，返回0
    if (m_channels.isEmpty()) {
        QMessageBox::warning(this, tr("加载错误"), tr("无法加载任何通道数据"));
        return 0;
    }
    
    // 更新图像列表
    updateImageList();
    
    // 默认选择第一个通道和图像
    if (ui->ChannelBox->itemCount() > 0) {
        ui->ChannelBox->setChecked(0, true);
    }
    
    if (ui->ImageBox->itemCount() > 0) {
        ui->ImageBox->setChecked(0, true);
    }
    
    // 更新显示
    updateMarkers();
    
    return m_channels.size();
}

QString ResultWidget::extractChannelLabel(const QString& filePath)
{
    QFileInfo info(filePath);
    QString fileName = info.fileName();
    QStringList parts = fileName.split("#");
    if (parts.size() < 2) return fileName;
    
    QStringList subParts = parts[1].split(".");
    return subParts[0];
}

void ResultWidget::updateImageList()
{
    // 先断开信号连接，避免更新列表时触发不必要的更新
    disconnect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    
    // 清空列表
    ui->ImageBox->removeAllItems();
    
    if (!m_channels.isEmpty()) {
        int imageNum = m_channels[0].size();
        for (int i = 0; i < imageNum; i++) {
            ui->ImageBox->addItem(QString::number(i + 1), i);
        }
        
        // 默认选中第一个图像
        if (ui->ImageBox->itemCount() > 0) {
            ui->ImageBox->setChecked(0, true);
        }
    }
    
    // 重新连接信号
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
}

void ResultWidget::clear()
{
    // 清空数据
    m_channels.clear();
    
    // 清空UI
    ui->ChannelBox->removeAllItems();
    ui->ImageBox->removeAllItems();
    
    // 清空图像显示
    QList<QImage> emptyList;
    ui->contentWidget->setImages(emptyList);
}

void ResultWidget::setRowNum(int row)
{
    ui->contentWidget->setRowNum(row);
}

void ResultWidget::setColNum(int col)
{
    ui->contentWidget->setColNum(col);
}

void ResultWidget::setHeight(int height)
{
    ui->contentWidget->setHeight(height);
}

void ResultWidget::setWidth(int width)
{
    ui->contentWidget->setWidth(width);
}

void ResultWidget::updateMarkers()
{
    // 检查是否有数据
    if (m_channels.isEmpty()) return;
    
    // 获取选中的通道和图像
    QList<QVariant> checkedChannels = ui->ChannelBox->values(QCheckComboBox::CHECKED);
    QList<QVariant> checkedImages = ui->ImageBox->values(QCheckComboBox::CHECKED);
    
    // 如果没有选中任何通道或图像，清空显示
    if (checkedChannels.isEmpty() || checkedImages.isEmpty()) {
        QList<QImage> emptyList;
        ui->contentWidget->setImages(emptyList);
        return;
    }
    
    // 准备图像列表
    QList<QImage> images;
    images.reserve(checkedChannels.size() * checkedImages.size()); // 预分配内存
    
    for (const QVariant& channel : checkedChannels) {
        int channelIndex = channel.toInt();
        if (channelIndex < 0 || channelIndex >= m_channels.size()) continue;
        
        for (const QVariant& image : checkedImages) {
            int imageIndex = image.toInt();
            if (imageIndex < 0 || imageIndex >= m_channels[channelIndex].size()) continue;
            
            images.push_back(m_channels[channelIndex][imageIndex]);
        }
    }
    
    // 更新显示
    ui->contentWidget->setImages(images);
}

