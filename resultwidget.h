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

    int loadMrdFiles(QString fpath); // path is any channel file path, but all files need to be in the same folder
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
