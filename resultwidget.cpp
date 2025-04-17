#include "../resultwidget.h"
#include "ui_resultwidget.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsView>

#include "mrdparser.h"

namespace {
QStringList getAllChannelsFile(QString path){
    QFileInfo fileInfo(path);
    if(!fileInfo.exists()){
        qWarning() << "File path does not exist: " << path;
        return {};
    }

    auto dir = fileInfo.absoluteDir();
    auto fileName = fileInfo.fileName();

    static QRegularExpression namePattern("^(.*)#(\\d+)\\.(\\w+)$");
    QRegularExpressionMatch match = namePattern.match(fileName);
    if (!match.hasMatch()) {
        qWarning() << "Invalid file name pattern, expected format: prefix#number.suffix";
        return {};
    }

    QString prefix = QRegularExpression::escape(match.captured(1));
    QString suffix = QRegularExpression::escape(match.captured(3));

    const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
    auto newPattern = QString("^%1#\\d+\\.%2$").arg(prefix, suffix);
    auto result = files.filter(QRegularExpression(newPattern));
    for(auto& fn:result){
        fn = dir.filePath(fn);
    }
    return result;
}

} // namespace


ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ResultWidget)
{
    ui->setupUi(this);
    ui->ChannelBox->setMinimumWidth(60);
    ui->ImageBox->setMinimumWidth(80);

    connect(ui->ChannelBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);
    connect(ui->ImageBox, &QCheckComboBox::itemStatusChanged, this, &ResultWidget::updateMarkers);

    connect(ui->rowSpin, &QSpinBox::valueChanged, this, &ResultWidget::setRowNum);
    connect(ui->columnSpin, &QSpinBox::valueChanged, this, &ResultWidget::setColNum);
    connect(ui->widthSpin, &QSpinBox::valueChanged, this, &ResultWidget::setWidth);
    connect(ui->heightSpin, &QSpinBox::valueChanged, this, &ResultWidget::setHeight);


    ui->contentWidget->init(
        ui->rowSpin->value(),
        ui->columnSpin->value(),
        ui->widthSpin->value(),
        ui->heightSpin->value()
        );
}

ResultWidget::~ResultWidget()
{
    delete ui;
}

int ResultWidget::loadMrdFiles(QString fpath)
{    
    auto files = getAllChannelsFile(fpath);

    for(const auto& file:files){
        auto content = MrdParser::parseFile(file);
        m_channels.emplace_back(MrdParser::reconImages(content));

        // 更新通道列表
        auto label = file.split("#")[1].split(".")[0];
        ui->ChannelBox->addItem(label, m_channels.length()-1);
    }

    // 更新图片列表
    auto imageNum = m_channels[0].length();
    for(int i=0;i<imageNum;i++){
        ui->ImageBox->addItem(QString::number(i+1), i);
    }

    return files.size();
}

void ResultWidget::clear()
{
    m_channels.clear();
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
    auto checkedChannels = ui->ChannelBox->values(QCheckComboBox::CHECKED);
    auto checkedImages = ui->ImageBox->values(QCheckComboBox::CHECKED);

    QList<QImage> images;
    for(int i=0;i<checkedChannels.length();i++){
        for(int j=0;j<checkedImages.length();j++){
            auto channelIndex = checkedChannels[i].toInt();
            auto imageIndex = checkedImages[j].toInt();
            images.push_back(m_channels[channelIndex][imageIndex]);
        }
    }
    ui->contentWidget->setImages(images);
}

