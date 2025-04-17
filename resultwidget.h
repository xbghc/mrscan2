#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QGraphicsScene>
#include <QGridLayout>
#include <QImage>
#include <QList>
#include <QWidget>

namespace Ui{
class ResultWidget;
}

class ResultWidget : public QWidget
{
    Q_OBJECT

public:
    ResultWidget(QWidget *parent = nullptr);
    ~ResultWidget();

    int loadMrdFiles(QString path); // path是任意一个通道文件的路径，但是所有文件都需要在同一文件夹中
    void clear();

public slots:
    void setRowNum(int row);
    void setColNum(int col);
    void setHeight(int height);
    void setWidth(int width);

private:
    QList<QList<QImage>> m_channels;

    Ui::ResultWidget* ui;

    void updateMarkers();
};
#endif // RESULTWIDGET_H
