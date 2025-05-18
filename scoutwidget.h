#ifndef SCOUTWIDGET_H
#define SCOUTWIDGET_H

#include <QJsonObject>
#include <QVector3D>
#include <QMouseEvent>

#include "qimageswidget.h"

class ScoutWidget : public QImagesWidget
{
    Q_OBJECT
public:
    ScoutWidget(QWidget* parent=nullptr);

    void setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles, QList<QVector3D> offsets);
    void updateMarkers() override;

    void preview(double fov, double thickness, double sliceSeparation, int noSlices, QVector3D angles, QVector3D offsets);
    void preview(double fov, double thickness, int noSlices, QList<QVector3D> angles, QList<QVector3D> offsets);

    void setFov(double fov);
    void clearLines();

signals:
    void offsetChanged(QVector3D movement); /// 返回变化的量，单位是现实长度单位，与offset一致
    void angleChanged(QVector3D movement); /// 返回角度变化量

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QVector3D> m_angles;
    QList<QVector3D> m_offsets;
    double m_fov;
    int m_rowNum=3;
    int m_colNum=3;

    /// row, col -> axis(0:x,1:y,2:y)
    QVector<std::pair<QVector3D, QVector3D>> m_axesMap;
    QPointF m_prevMousePos;

    void previewSlice(double fov, QVector3D angles, QVector3D offsets);

    void onViewMousePressd(int row, int col, QMouseEvent *event);
    void onViewMouseMoved(int row, int col, QMouseEvent *event);
    void onViewWheeled(int row, int col, QWheelEvent *event);
};

#endif // SCOUTWIDGET_H
