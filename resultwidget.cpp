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

#include "mrdparser.h"

namespace {
// Get all channel files from the given path
QStringList getAllChannelsFile(const QString& path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        LOG_WARNING(QString("File path does not exist: %1").arg(path));
        return {};
    }

    QDir dir = fileInfo.absoluteDir();
    QString fileName = fileInfo.fileName();

    static QRegularExpression namePattern("^(.*)#(\\d+)\\.(\\w+)$");
    QRegularExpressionMatch match = namePattern.match(fileName);
    if (!match.hasMatch()) {
        LOG_WARNING("Invalid filename format, expected format: prefix#number.suffix");
        return {};
    }

    QString prefix = QRegularExpression::escape(match.captured(1));
    QString suffix = QRegularExpression::escape(match.captured(3));

    // Build new regex to match related files
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

int ResultWidget::loadMrdFiles(QString fpath)
{    
    // Clear old data
    clear();
    
    // Get all channel files
    QStringList files = getAllChannelsFile(fpath);
    if (files.isEmpty()) {
        QMessageBox::warning(this, tr("Loading Error"), tr("No valid channel files found: %1").arg(fpath));
        return 0;
    }
    
    // Show loading progress
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // Load all channel files
    for (const auto& file : files) {
        try {
            // Parse file
            auto content = MrdParser::parseFile(file);
            if (!content) {
                LOG_WARNING(QString("File parsing failed: %1").arg(file));
                continue;
            }
            
            // Reconstruct images
            auto images = MrdParser::reconImages(content.get());
            if (images.isEmpty()) {
                LOG_WARNING(QString("Image reconstruction failed: %1").arg(file));
                continue;
            }
            
            m_channels.push_back(images[0]);
            
            // Update channel list, extract channel number
            QString label = extractChannelLabel(file);
            ui->ChannelBox->addItem(label, m_channels.size() - 1);
        } catch (const std::exception& e) {
            LOG_ERROR(QString("Error processing file: %1, Error: %2").arg(file).arg(e.what()));
        }
    }
    
    // Restore cursor
    QApplication::restoreOverrideCursor();
    
    // If no channels were loaded successfully, return 0
    if (m_channels.isEmpty()) {
        QMessageBox::warning(this, tr("Loading Error"), tr("Unable to load any channel data"));
        return 0;
    }
    
    // Update image list
    updateImageList();
    
    // Select first channel and image by default
    if (ui->ChannelBox->itemCount() > 0) {
        ui->ChannelBox->setChecked(0, true);
    }
    
    if (ui->ImageBox->itemCount() > 0) {
        ui->ImageBox->setChecked(0, true);
    }
    
    // Update display
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

