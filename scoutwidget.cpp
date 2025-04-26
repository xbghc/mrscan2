#include "scoutwidget.h"
#include <QMatrix4x4>
#include <QPen>
#include "utils.h"
#include <memory>

namespace {
// Compare if two floating point numbers are equal
bool equal(float a, float b, float epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Calculate normal vector based on Euler angles
QVector3D getVector(const QVector3D& angles) {
    QMatrix4x4 r;
    r.rotate(angles.x(), QVector3D(1, 0, 0));
    r.rotate(angles.y(), QVector3D(0, 1, 0));
    r.rotate(angles.z(), QVector3D(0, 0, 1));

    return r.map(QVector3D(0, 0, 1));
}

// Check if vector is parallel to coordinate axes, if so return the corresponding unit vector
QVector3D getAxis(const QVector3D& angles) {
    QVector3D v = getVector(angles);
    QVector3D result(0, 0, 0);
    
    // Check if parallel to x-axis
    if (equal(std::abs(v.x()), 1.0f)) {
        result.setX(v.x() > 0 ? 1 : -1);
        return result;
    } 
    // Check if parallel to y-axis
    else if (equal(std::abs(v.y()), 1.0f)) {
        result.setY(v.y() > 0 ? 1 : -1);
        return result;
    } 
    // Check if parallel to z-axis
    else if (equal(std::abs(v.z()), 1.0f)) {
        result.setZ(v.z() > 0 ? 1 : -1);
        return result;
    }
    
    LOG_WARNING("Scout normal vector angle error, must be parallel to x, y or z axis");
    return result;
}

QList<QPointF> getLineInViewport(double a, double b, double c, double viewFov, double lineFov){
    // ax + by + c = 0
    QList<QPointF> points;

    if(a == 0){
        // 1. When slice is parallel to y (a=0), we have by + c = 0, so y=-c/b
        auto y = -c/b;
        if(abs(y) > viewFov/2){
            LOG_WARNING("Line out of viewport");
        }
        points.push_back(QPointF(-viewFov/2, y));
        points.push_back(QPointF(viewFov/2, y));
    }else{
        // 2. Given y, x = (c-by)/a; Given x, y = (c-ax)/b
        //
        // Assume y equals ±fov/2
        //
        auto y1 = viewFov/2;
        auto y2 = -viewFov/2;
        auto x1 = (c-b*y1)/a;
        auto x2 = (c-b*y2)/a;

        if(x1 > viewFov/2){
            x1 = viewFov/2;
            y1 = (c-(a*x1))/b;
        }else if(x1 < -viewFov/2){
            x1 = -viewFov/2;
            y1 = (c-(a*x1))/b;
        }
        if(abs(y1) > viewFov/2){
            LOG_WARNING("Line is outside scout range");
        }
        points.push_back(QPointF(x1, y1));

        if(x2 > viewFov/2){
            x2 = viewFov/2;
            y2 = (c-(a*x2))/b;
        }else if(x2 < -viewFov/2){
            x2 = -viewFov/2;
            y2 = (c-(a*x1))/b;
        }
        if(abs(y2) > viewFov/2){
            LOG_WARNING("Line is outside scout range");
        }
        points.push_back(QPointF(x2, y2));
    }

    if(points.length() != 2){
        LOG_WARNING("Intersection line calculation error");
    }

    return points;
}

} // namespace

ScoutWidget::ScoutWidget(QWidget *parent)
    : QImagesWidget(parent)
{
    setRowNum(3);
    setColNum(3);
    setImages(QList<QImage>());
}

void ScoutWidget::setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles, QList<QVector3D> offsets)
{
    QImagesWidget::setImages(images);
    m_fov = fov;
    m_angles = angles;
    m_offsets = offsets;
}

void ScoutWidget::updateMarkers()
{
    QImagesWidget::updateMarkers();
}

void ScoutWidget::preview(double fov, double thickness, double sliceSeparation, int noSlices, QVector3D angles, QVector3D offsets)
{
    auto angleList = QList<QVector3D>();
    auto offsetList = QList<QVector3D>();
    auto v = getVector(angles);


    for(int i=0;i<noSlices;i++){
        double o=(i - static_cast<double>(noSlices) + 1) * sliceSeparation;
        offsetList.push_back(o * v + offsets);
        angleList.push_back(angles);
    }
    preview(fov, thickness, noSlices, angleList, offsetList);
}

void ScoutWidget::preview(double fov, double thickness, int noSlices, QList<QVector3D> angles, QList<QVector3D> offsets)
{
    for(int i=0;i<noSlices;i++){
        previewSlice(fov, angles[i], offsets[i]);
    }
}

void ScoutWidget::setFov(double fov)
{
    m_fov = fov;
}

void ScoutWidget::clearLines()
{
    // Clear all previously added lines, but keep the images
    // QImagesWidget doesn't have a direct clearLines method, but we can refresh the scene by calling updateMarkers()
    // updateMarkers() will clear the scene (scene->clear()) and re-add the images
    updateMarkers();
}

void ScoutWidget::previewSlice(double fov, QVector3D angles, QVector3D offsets)
{
    // TODO Function code can be optimized

    for(int i=0;i<m_angles.length();i++){
        /* Ignore FOV here, view the slice as a plane
         * First calculate the normal vector v of the slice based on angles, v[0]*x+v[1]*y+v[2]*z+c=0
         * Scout has an offset relative to the origin, viewed as center in graphicScene, equivalent to slice offset in opposite direction, so slice offset is offsets - m_offsets[i]
         * Scout is parallel to a coordinate axis, so one of x,y,z equals 0, thus we have ax + by + c = 0, where x and y have different meanings than in 3D
         * The x and y discussed below are in the scout plane rather than in 3D
         * Scout value range is (-m_fov/2, m_fov/2)
        */
        auto v = getVector(angles);
        auto c = (m_offsets[i][0] - offsets[0])*v[0] + (m_offsets[i][1] - offsets[1])*v[1] + (m_offsets[i][2] - offsets[2])*v[2];
        auto axis = getAxis(m_angles[i]);

        QList<QPointF> points;
        // Normal vectors of 1 and -1 affect the result
        if(axis[0] != 0){
            points = getLineInViewport(v[1] * axis[0], v[2] * axis[0], c, m_fov, fov);
        }else if(axis[1] != 0){
            points = getLineInViewport(v[0] * axis[1], v[2] * axis[1], c, m_fov, fov);
        }else if(axis[2] != 0){
            points = getLineInViewport(v[0] * axis[2], v[1] * axis[2], c, m_fov, fov);
        }

        auto line = new QGraphicsLineItem(points[0].x(), points[0].y(), points[1].x(), points[1].y());
        QPen pen(Qt::red);      // Color
        pen.setWidth(2);        // Line width
        pen.setStyle(Qt::DashLine); // Dashed line
        line->setPen(pen);
        QImagesWidget::addLine(i, line);
    }
}
