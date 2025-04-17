#include "scoutwidget.h"
#include <QMatrix4x4>
#include <QPen>
#include "utils.h"

namespace {
// 比较两个浮点数是否相等
bool equal(float a, float b, float epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// 根据欧拉角计算法向量
QVector3D getVector(const QVector3D& angles) {
    QMatrix4x4 r;
    r.rotate(angles.x(), QVector3D(1, 0, 0));
    r.rotate(angles.y(), QVector3D(0, 1, 0));
    r.rotate(angles.z(), QVector3D(0, 0, 1));

    return r.map(QVector3D(0, 0, 1));
}

// 判断向量是否平行于坐标轴，如果是返回对应的单位向量
QVector3D getAxis(const QVector3D& angles) {
    QVector3D v = getVector(angles);
    QVector3D result(0, 0, 0);
    
    // 检查是否平行于x轴
    if (equal(std::abs(v.x()), 1.0f)) {
        result.setX(v.x() > 0 ? 1 : -1);
        return result;
    } 
    // 检查是否平行于y轴
    else if (equal(std::abs(v.y()), 1.0f)) {
        result.setY(v.y() > 0 ? 1 : -1);
        return result;
    } 
    // 检查是否平行于z轴
    else if (equal(std::abs(v.z()), 1.0f)) {
        result.setZ(v.z() > 0 ? 1 : -1);
        return result;
    }
    
    LOG_WARNING("scout的法向量角度错误，必须与x、y或z轴平行");
    return result;
}

QList<QPointF> getLineInViewport(double a, double b, double c, double viewFov, double lineFov){
    // ax + by + c = 0
    QList<QPointF> points;

    if(a == 0){
        // 1. 当切片与y平行(a=0)，有by + c = 0, 则y=-c/b
        auto y = -c/b;
        if(abs(y) > viewFov/2){
            qDebug() << "line out of viewport";
        }
        points.push_back(QPointF(-viewFov/2, y));
        points.push_back(QPointF(viewFov/2, y));
    }else{
        // 2. 已知y, x = (c-by)/a; 已知x, y = (c-ax)/b
        //
        // 假设y为±fov/2
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
            qDebug() << "直线在scout范围之外";
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
            qDebug() << "直线在scout范围之外";
        }
        points.push_back(QPointF(x2, y2));
    }

    if(points.length() != 2){
        qDebug() << "交线计算出错";
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
    // 清除之前所有添加的线条，但保留图像
    // QImagesWidget没有直接的clearLines方法，但我们可以通过调用updateMarkers()来刷新场景
    // updateMarkers()会清除场景(scene->clear())并重新添加图像
    updateMarkers();
}
void ScoutWidget::previewSlice(double fov, QVector3D angles, QVector3D offsets)
{
    // TODO 函数代码可以优化

    for(int i=0;i<m_angles.length();i++){
        /* 这里无视FOV，将切片视作平面
         * 先根据angles求出切片的法向量v，v[0]*x+v[1]*y+v[2]*z+c=0
         * scout相对于原点有偏移，在graphicScene中视为中心，相当于slice朝反方向偏移，所以slice的偏移为offsets - m_offsets[i]
         * scout与某个坐标轴平行，所以x,y,z中某个值为0，于是有ax + by + c = 0，这个x和y与3维中的xy含义不同
         * 下面的xy讨论的都是scout平面中的xy而非3维中的xy
         * scout的取值范围为(-m_fov/2, m_fov/2)
        */
        auto v = getVector(angles);
        auto c = (m_offsets[i][0] - offsets[0])*v[0] + (m_offsets[i][1] - offsets[1])*v[1] + (m_offsets[i][2] - offsets[2])*v[2];
        auto axis = getAxis(m_angles[i]);

        QList<QPointF> points;
        // 法向量为1和-1对结果是有影响的
        if(axis[0] != 0){
            points = getLineInViewport(v[1] * axis[0], v[2] * axis[0], c, m_fov, fov);
        }else if(axis[1] != 0){
            points = getLineInViewport(v[0] * axis[1], v[2] * axis[1], c, m_fov, fov);
        }else if(axis[2] != 0){
            points = getLineInViewport(v[0] * axis[2], v[1] * axis[2], c, m_fov, fov);
        }

        auto line = new QGraphicsLineItem(points[0].x(), points[0].y(), points[1].x(), points[1].y());
        QPen pen(Qt::red);      // 颜色
        pen.setWidth(2);        // 线宽
        pen.setStyle(Qt::DashLine); // 虚线
        line->setPen(pen);
        QImagesWidget::addLine(i, line);
        // qDebug() << line.line();
    }
}
