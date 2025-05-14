#include "../resultwidget.h"
#include "ui_resultwidget.h"
#include "utils.h"
#include "mrdparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsView>
#include <QMessageBox>
#include <QApplication>
#include <QRegularExpression>
#include <memory>


ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::ResultWidget>())
{
    ui->setupUi(this);
    initializeUI();
    setupConnections();
}

ResultWidget::~ResultWidget()
{
    clear();
}

void ResultWidget::initializeUI()
{
    // Set initial configuration for UI components
    ui->ChannelBox->setMinimumWidth(60);
    ui->ImageBox->setMinimumWidth(80);
    
    // Initialize image display area
    ui->contentWidget->init(
        ui->rowSpin->value(),
        ui->columnSpin->value(),
        ui->widthSpin->value(),
        ui->heightSpin->value()
    );
}

void ResultWidget::setupConnections()
{
    // Connect selection box change signals
    connect(ui->ChannelBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    
    // Connect layout control change signals
    connect(ui->rowSpin, &QSpinBox::valueChanged, this, &ResultWidget::setRowNum);
    connect(ui->columnSpin, &QSpinBox::valueChanged, this, &ResultWidget::setColNum);
    connect(ui->widthSpin, &QSpinBox::valueChanged, this, &ResultWidget::setWidth);
    connect(ui->heightSpin, &QSpinBox::valueChanged, this, &ResultWidget::setHeight);
}

void ResultWidget::setData(const Exam& exam)
{
    /// @todo 或许exam中的response应该有一个提供标签的方法，
    /// 毕竟可能是通道1，2，4而不是按顺序
    clear();
    m_channels = exam.images();
    ui->ChannelBox->setItems(QStringList(m_channels.size()));
    updateImageList();
    updateMarkers();
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
    // Disconnect signals first to avoid unnecessary updates when updating the list
    disconnect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    
    // Clear list
    ui->ImageBox->removeAllItems();
    
    if (!m_channels.isEmpty()) {
        int imageNum = m_channels[0].size();
        for (int i = 0; i < imageNum; i++) {
            ui->ImageBox->addItem(QString::number(i + 1), i);
        }
        
        // Select first image by default
        if (ui->ImageBox->itemCount() > 0) {
            ui->ImageBox->setChecked(0, true);
        }
    }
    
    // Reconnect signals
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
}

void ResultWidget::clear()
{
    // Clear data
    m_channels.clear();
    
    // Clear UI
    ui->ChannelBox->removeAllItems();
    ui->ImageBox->removeAllItems();
    
    // Clear image display
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
    // Check if there is data
    if (m_channels.isEmpty()) return;
    
    // Get selected channels and images
    QList<QVariant> checkedChannels = ui->ChannelBox->values(QCheckComboBox::CHECKED);
    QList<QVariant> checkedImages = ui->ImageBox->values(QCheckComboBox::CHECKED);
    
    // If no channels or images are selected, clear display
    if (checkedChannels.isEmpty() || checkedImages.isEmpty()) {
        QList<QImage> emptyList;
        ui->contentWidget->setImages(emptyList);
        return;
    }
    
    // Prepare image list
    QList<QImage> images;
    images.reserve(checkedChannels.size() * checkedImages.size()); // Preallocate memory
    
    for (const QVariant& channel : checkedChannels) {
        int channelIndex = channel.toInt();
        if (channelIndex < 0 || channelIndex >= m_channels.size()) continue;
        
        for (const QVariant& image : checkedImages) {
            int imageIndex = image.toInt();
            if (imageIndex < 0 || imageIndex >= m_channels[channelIndex].size()) continue;
            
            images.push_back(m_channels[channelIndex][imageIndex]);
        }
    }
    
    // Update display
    ui->contentWidget->setImages(images);
}

