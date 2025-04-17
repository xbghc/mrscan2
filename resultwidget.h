#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QGraphicsScene>
#include <QGridLayout>
#include <QImage>
#include <QList>
#include <QWidget>
#include <QVector>

namespace Ui{
class ResultWidget;
}

class ResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResultWidget(QWidget *parent = nullptr);
    ~ResultWidget();

    int loadMrdFiles(QString fpath); // path是任意一个通道文件的路径，但是所有文件都需要在同一文件夹中
    void clear();

public slots:
    void setRowNum(int row);
    void setColNum(int col);
    void setHeight(int height);
    void setWidth(int width);
    void updateMarkers();

private:
    QVector<QVector<QImage>> m_channels;

    Ui::ResultWidget* ui;

    void initializeUI();
    void setupConnections();
    QString extractChannelLabel(const QString& filePath);
    void updateImageList();
};
#endif // RESULTWIDGET_H
