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

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_sizeBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_pushButton_7_clicked();

private:
    void updateLayout();
    void loadImagesFromFile(QJsonObject patient, QJsonObject exam);
    void rotate(int angle);

    Ui::ImageGridWidget *ui;
    std::vector<QLabel*> imageLabels;
    int imagesPerRow;
    QSize imageSize;
};

#endif // IMAGEGRIDWIDGET_H
