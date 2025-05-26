#ifndef SCOUTWIDGET_H
#define SCOUTWIDGET_H

#include <QJsonObject>
#include <QMouseEvent>
#include <QVector3D>

#include "qimageswidget.h"

class ScoutWidget : public QImagesWidget {
    Q_OBJECT
public:
    ScoutWidget(QWidget *parent = nullptr);

    void setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles,
                        QList<QVector3D> offsets);
    void updateMarkers() override;

    void preview(double fov, double thickness, double sliceSeparation,
                 int noSlices, QVector3D angles, QVector3D offsets);
    void preview(double fov, double thickness,
                 QVector<QPair<QVector3D, QVector3D>> slices);

    void setScoutFov(double other);

    /**
   * @brief Given angle, get the corresponding rotation matrix
   * @details Rotation is in fixed x,y,z order
   * @param angle QVector3D, corresponding to rotation angles of x,y,z axes respectively
   * @return Rotation matrix
   */
    QMatrix4x4 rotateMatrix(const QVector3D angle) const;

    /**
   * @brief Get the intersection line of two planes
   * @return QPair<QVector3D point, QVector3D vector>
   * point A point on the intersection line
   * vector Direction of the intersection line
   * If vector length is 0, it means the two planes are parallel and have no intersection line
   */
    QPair<QVector3D, QVector3D> intersectionLine(const double A1, const double B1,
                                                 const double C1, const double D1,
                                                 const double A2, const double B2,
                                                 const double C2,
                                                 const double D2) const;

    QPair<QVector3D, QVector3D> intersectionLine(const QVector3D angle1,
                                                 const QVector3D offset1,
                                                 const QVector3D offset2,
                                                 const QVector3D angle2) const;

    /**
   *  @param scout Angle passed in during scout scanning
   *  @return [haxis, vaxis], spatial directions represented by right and down in the image respectively
   *  @note When the initial plane of the spectrometer changes, this function needs to be adjusted
   */
    std::pair<QVector3D, QVector3D> getViewAxes(QVector3D angle) const;


signals:
    void offsetChanged(
        QVector3D movement); /// Return the amount of change, unit is real length unit, consistent with offset
    void angleChanged(QVector3D movement); /// Return angle change amount

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void onViewMousePressd(int row, int col, QMouseEvent *event);
    void onViewMouseMoved(int row, int col, QMouseEvent *event);
    void onViewWheeled(int row, int col, QWheelEvent *event);

private:
    QVector<QPair<QVector3D, QVector3D>> m_scoutSlices;
    double m_scoutFov;
    int m_rowNum = 3;
    int m_colNum = 3;
    QPointF m_prevMousePos;

    QVector3D m_initNormalVector;
    QVector3D m_initHorizontalVector;
    QVector3D m_initVerticalVector;
    
    /**
   * @brief Preview a single slice, this method will not clear existing content in the view
   * @param fov FOV of the slice to be added
   * @param angles Angle of the slice
   * @param offsets Offset of the slice
   */
    void previewSlice(double fov, QVector3D angles, QVector3D offsets);
};

#endif // SCOUTWIDGET_H
