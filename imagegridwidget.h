#ifndef IMAGEGRIDWIDGET_H
#define IMAGEGRIDWIDGET_H

#include <QWidget>
#include <QImage>
#include <QJsonObject>
#include <QLabel>
#include <vector>

namespace Ui {
class ImageGridWidget;
}

class ImageGridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageGridWidget(QWidget *parent = nullptr, int imagesPerRow = 3);
    ~ImageGridWidget();

    void addImage(const QImage& image);
    void setImagesPerRow(int count);
    void clear();
    void setImageSize(const QSize& size);

private:
    void updateLayout();
    void loadImagesFromFile(QJsonObject patient, QJsonObject exam);

    Ui::ImageGridWidget *ui;
    std::vector<QLabel*> imageLabels;
    int imagesPerRow;
    QSize imageSize;
};

#endif // IMAGEGRIDWIDGET_H
