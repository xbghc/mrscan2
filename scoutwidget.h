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
    void valueChanged(QVector3D angles, QVector3D offsets);
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QList<QVector3D> m_angles;
    QList<QVector3D> m_offsets;
    double m_fov;

    void previewSlice(double fov, QVector3D angles, QVector3D offsets);

    void onViewMousePress(int row, int col, QMouseEvent* event);
    void onViewMouseMove(int row, int col, QMouseEvent* event);
    void onViewWheeled(int row, int col, QWheelEvent* event);
};

#endif // SCOUTWIDGET_H
