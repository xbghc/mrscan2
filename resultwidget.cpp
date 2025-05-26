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
#include <memory>


ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::ResultWidget>())
{
    ui->setupUi(this);
    
    setupConnections();

    initializeUI();

    ui->contentWidget->setUpdateEnabled(false);
    ui->contentWidget->setRowNum(ui->rowSpin->value());
    ui->contentWidget->setColNum(ui->columnSpin->value());
    ui->contentWidget->setViewWidth(ui->widthSpin->value());
    ui->contentWidget->setViewHeight(ui->heightSpin->value());
    ui->contentWidget->setUpdateEnabled(true);
    // 不需要手动更新，主窗口初始化时会调整尺寸，触发contentWidget的resizeEvent
}

ResultWidget::~ResultWidget()
{
    clear();
}

void ResultWidget::initializeUI()
{
    // 设置spinbox的最小宽度
    ui->columnSpin->setMinimumWidth(60);
    ui->rowSpin->setMinimumWidth(60);
    ui->heightSpin->setMinimumWidth(80);
    ui->widthSpin->setMinimumWidth(80);
}

void ResultWidget::setupConnections()
{
    // Connect selection box change signals
    connect(ui->ChannelBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateImages);
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateImages);
    
    // Connect layout control change signals
    connect(ui->rowSpin, &QSpinBox::valueChanged, ui->contentWidget, &QImagesWidget::setRowNum);
    connect(ui->columnSpin, &QSpinBox::valueChanged, ui->contentWidget, &QImagesWidget::setColNum);
    connect(ui->widthSpin, &QSpinBox::valueChanged, ui->contentWidget, &QImagesWidget::setViewWidth);
    connect(ui->heightSpin, &QSpinBox::valueChanged, ui->contentWidget, &QImagesWidget::setViewHeight);
}

void ResultWidget::setData(const Exam& exam)
{
    clear();
    m_channels = exam.images();

    // 更新选择框
    auto channelsNum = m_channels.size();
    auto imagesNum = m_channels[0].size();
    QStringList channelsList;   
    QStringList imagesList;
    for(int i=0;i<channelsNum;i++){
        channelsList.append(QString::number(i));
    }
    for(int i=0;i<imagesNum;i++){
        imagesList.append(QString::number(i));
    }
    ui->ChannelBox->setItems(channelsList);
    ui->ImageBox->setItems(imagesList);
    
    // 选中所有
    for(int i=0;i<channelsNum;i++){
        ui->ChannelBox->setChecked(i, true);
    }
    for(int i=0;i<imagesNum;i++){
        ui->ImageBox->setChecked(i, true);
    }

    updateImages();
}

void ResultWidget::clear()
{
    // Clear data
    QVector<QVector<QImage>>().swap(m_channels);
    
    // Clear UI
    ui->ChannelBox->removeAllItems();
    ui->ImageBox->removeAllItems();
    
    // Clear image display
    ui->contentWidget->setImages(QList<QImage>());
}

void ResultWidget::updateImages()
{
    if (m_channels.isEmpty()) return;
    
    // Get selected channels and images
    QList<QVariant> checkedChannels = ui->ChannelBox->values(QCheckComboBox::CHECKED);
    QList<QVariant> checkedImages = ui->ImageBox->values(QCheckComboBox::CHECKED);
    
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

    ui->contentWidget->setImages(images);
}
