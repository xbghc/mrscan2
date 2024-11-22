#include "imagegridwidget.h"
#include "ui_imagegridwidget.h"

#include <QDataStream>
#include <QFile>
#include <QJsonDocument>


ImageGridWidget::ImageGridWidget(QWidget *parent, int imagesPerRow)
    : QWidget(parent)
    , ui(new Ui::ImageGridWidget)
    , imagesPerRow(imagesPerRow)
    , imageSize(200, 200)
{
    ui->setupUi(this);
}

ImageGridWidget::~ImageGridWidget()
{
    delete ui;
}

void ImageGridWidget::addImage(const QImage& image) {
    auto pixmap = QPixmap::fromImage(image);
    QLabel* label = new QLabel();
    label->setFixedSize(imageSize);
    label->setScaledContents(true);
    label->setPixmap(pixmap.scaled(imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);

    imageLabels.push_back(label);
    updateLayout();
}

void ImageGridWidget::setImagesPerRow(int count) {
    if (count > 0 && count != imagesPerRow) {
        imagesPerRow = count;
        updateLayout();
    }
}

void ImageGridWidget::clear() {
    for (auto label : imageLabels) {
        delete label;
    }
    imageLabels.clear();
    updateLayout();
}

void ImageGridWidget::setImageSize(const QSize& size) {
    imageSize = size;
    for (auto label : imageLabels) {
        label->setFixedSize(imageSize);
        label->setPixmap(label->pixmap().scaled(imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    updateLayout();
}

void ImageGridWidget::updateLayout() {
    // Remove all items from grid
    QGridLayout* gridLayout = ui->scrollAreaWidgetContents->findChild<QGridLayout*>();
    while (gridLayout->count() > 0) {
        QLayoutItem* item = gridLayout->takeAt(0);
        if (item->widget()) {
            gridLayout->removeWidget(item->widget());
        }
        delete item;
    }

    // Add images to grid
    for (size_t i = 0; i < imageLabels.size(); ++i) {
        int row = i / imagesPerRow;
        int col = i % imagesPerRow;
        gridLayout->addWidget(imageLabels[i], row, col);
    }
}

void ImageGridWidget::rotate(int angle)
{
    if(angle<0 || angle>360){
        qDebug() << "invalid angle: " << angle;
        return;
    }

    for(size_t i=0;i<imageLabels.size();i++){
        QPixmap originalPixmap = imageLabels[i]->pixmap();
        QImage image = originalPixmap.toImage();

        QImage rotatedImage = image.transformed(QTransform().rotate(90));

        imageLabels[i]->setPixmap(QPixmap::fromImage(rotatedImage));
    }
}


void ImageGridWidget::on_pushButton_clicked()
{
    rotate(90);
}


void ImageGridWidget::on_pushButton_2_clicked()
{
    rotate(270);
}


void ImageGridWidget::on_sizeBox_valueChanged(int num)
{
    setImageSize(QSize(num,num));
}


void ImageGridWidget::on_spinBox_2_valueChanged(int num)
{
    setImagesPerRow(num);
}


void ImageGridWidget::on_pushButton_7_clicked()
{
    auto container = ui->scrollArea;
    int totalWidth = container->width();
    int width = totalWidth / imagesPerRow - 15;
    ui->sizeBox->setValue(width);
}

