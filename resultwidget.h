#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include "exam.h"

#include <QGraphicsScene>
#include <QGridLayout>
#include <QImage>
#include <QList>
#include <QVector>
#include <QWidget>
#include <memory>

namespace Ui {
class ResultWidget;
}

/**
 * @class ResultWidget
 * @brief 内置QImagesWidget，提供了可交互的接口
 * @details 接收一个Exam对象，展示Exam的相关数据(目前只实现了简单接口)
 */
class ResultWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultWidget(QWidget *parent = nullptr);
    ~ResultWidget();

    void setData(const Exam &exam);
    void clear();

public slots:
    void updateImages();

private:
    QVector<QVector<QImage>> m_channels;

    std::unique_ptr<Ui::ResultWidget> ui;

    void setupUi();
    void setupConnections();
};
#endif // RESULTWIDGET_H
