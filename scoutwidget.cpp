#include "scoutwidget.h"
#include "utils.h"
#include <QMatrix4x4>
#include <QPen>

namespace {
// Compare if two floating point numbers are equal
bool equal(float a, float b, float epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}
} // namespace

ScoutWidget::ScoutWidget(QWidget *parent) : QImagesWidget(parent) {
    setRowNum(m_rowNum);
    setColNum(m_colNum);
    setImages(QList<QImage>());
}

void ScoutWidget::setScoutImages(QList<QImage> images, double fov,
                                 QList<QVector3D> angles,
                                 QList<QVector3D> offsets) {
    QImagesWidget::setImages(images);
    m_scoutFov = fov;
    m_slices.clear();
    for(int i=0;i<angles.length();i++){
        m_slices.append(qMakePair(angles[i], offsets[i]));
    }
}

void ScoutWidget::updateMarkers() { QImagesWidget::updateMarkers(); }

std::pair<QVector3D, QVector3D> ScoutWidget::getViewAxes(QVector3D angle) {
    auto hAxis = INIT_HORIZONTAL_VECTOR;
    auto vAxis = INIT_VERTICAL_VECTOR;

    auto r = rotateMatrix(angle);

    hAxis = r.map(hAxis);
    vAxis = r.map(vAxis);

    return {hAxis, vAxis};
}

void ScoutWidget::preview(double fov, double thickness, double sliceSeparation,
                          int noSlices, QVector3D angles, QVector3D offsets) {
    if (m_slices.empty()) {
        return;
    }

    QVector<QPair<QVector3D, QVector3D>> slices;
    auto v = rotateMatrix(angles).map(INIT_NORMAL_VECTOR);
    for (int i = 0; i < noSlices; i++) {
        double o = (i - (static_cast<double>(noSlices) - 1) / 2) * sliceSeparation;
        auto offset = o * v + offsets;
        slices.append(qMakePair(angles, offset));
    }
    
    preview(fov, thickness, slices);
}


void ScoutWidget::preview(double fov, double thickness, QVector<QPair<QVector3D, QVector3D>> slices) {
    if (m_slices.empty()) {
        return;
    }

    updateMarkers();

    LOG_INFO("start paint");
    for (int i = 0; i < slices.length(); i++) {
        previewSlice(fov, slices[i].first, slices[i].second);
    }

    LOG_INFO("paint end");
}

void ScoutWidget::setScoutFov(double fov) { m_scoutFov = fov; }

bool ScoutWidget::eventFilter(QObject *watched, QEvent *event) {

    auto viewport = qobject_cast<QWidget *>(watched);
    if (!viewport) {
        return QWidget::eventFilter(watched, event);
    }
    auto [row, col] = viewPortPosition(viewport);

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        onViewMousePressd(row, col, static_cast<QMouseEvent *>(event));
        return false;
    case QEvent::MouseMove:
        onViewMouseMoved(row, col, static_cast<QMouseEvent *>(event));
        return false;
    case QEvent::Wheel:
        onViewWheeled(row, col, static_cast<QWheelEvent *>(event));
        return false;
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

QMatrix4x4 ScoutWidget::rotateMatrix(const QVector3D angle) const {
    QMatrix4x4 r;
    r.rotate(angle.x(), QVector3D(1, 0, 0));
    r.rotate(angle.y(), QVector3D(0, 1, 0));
    r.rotate(angle.z(), QVector3D(0, 0, 1));
    return r;
}

QPair<QVector3D, QVector3D>
ScoutWidget::intersectionLine(const double A1, const double B1, const double C1,
                              const double D1, const double A2, const double B2,
                              const double C2, const double D2) const {
    auto plane1NormalVector = QVector3D(A1, B1, C1);
    auto plane2NormalVector = QVector3D(A2, B2, C2);
    // 法向量相等说明平面平行
    if ((plane1NormalVector - plane2NormalVector).lengthSquared() < 1e-6) {
        return {QVector3D(), QVector3D(0, 0, 0)};
    }

    QVector3D point;
    auto lineVector =
        QVector3D::crossProduct(plane1NormalVector, plane2NormalVector);

    if (lineVector.x() > 1e-6) {
        point.setX(0);

        auto det = B1 * C2 - C1 * B2;

        if(det > 1e-6){
            point.setY((-C2 * D1 + C1 * D2) / det);
            point.setZ((B2 * D1 - B1 * D2) / det);
        }else{
            return {QVector3D(), QVector3D(0, 0, 0)};
        }
    } else {
        point.setY(0);

        auto det = A1 * C2 - C1 * A2;

        if(det > 1e-6){
            point.setX((-C2 * D1 + C1 * D2) / det);
            point.setZ((A2 * D1 - A1 * D2) / det);
        }else{
            return {QVector3D(), QVector3D(0, 0, 0)};
        }
    }
    return {point, lineVector};
}

QPair<QVector3D, QVector3D>
ScoutWidget::intersectionLine(const QVector3D angle1, const QVector3D offset1,
                              const QVector3D offset2,
                              const QVector3D angle2) const {
    auto v1 = rotateMatrix(angle1).map(INIT_NORMAL_VECTOR);
    auto v2 = rotateMatrix(angle2).map(INIT_NORMAL_VECTOR);
    auto D1 = -(QVector3D::dotProduct(v1, offset1));
    auto D2 = -(QVector3D::dotProduct(v2, offset2));
    return intersectionLine(v1.x(), v1.y(), v1.z(), D1, v2.x(), v2.y(), v2.z(),
                            D2);
}


void ScoutWidget::previewSlice(double fov, QVector3D angles,
                               QVector3D offsets) {
    /// @todo 可选择是否将slice视为无边界的平面

    for (int i = 0; i < m_slices.length(); i++) {
        auto scoutAngle = m_slices[i].first;
        auto scoutOffset = m_slices[i].second;

        auto [point, vector] =
            intersectionLine(scoutAngle, scoutOffset, angles, offsets);

        if (vector.lengthSquared() < 1e-6) {
            continue;
        }

        // 生成足够长的直线以绘制
        const double lineSize = 500;
        auto p1 = point - vector * lineSize;
        auto p2 = point + vector * lineSize;

        // 根据slice所在的平面绘制图片
        auto [hAxis, vAxis] = getViewAxes(scoutAngle);
        auto line = new QGraphicsLineItem(
            QVector3D::dotProduct(p1, hAxis), QVector3D::dotProduct(p1, vAxis),
            QVector3D::dotProduct(p2, hAxis), QVector3D::dotProduct(p2, vAxis));

        QPen pen(Qt::red);          // Color
        pen.setWidth(2);            // Line width
        pen.setStyle(Qt::DashLine); // Dashed line
        line->setPen(pen);
        QImagesWidget::addLine(i, line);
    }
}

void ScoutWidget::onViewMousePressd(int row, int col, QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }

    auto view = this->view(row, col);
    m_prevMousePos = view->mapToScene(event->pos());
}

void ScoutWidget::onViewMouseMoved(int row, int col, QMouseEvent *event) {
    auto [haxis, vaxis] = getViewAxes(m_slices[row * m_colNum + col].first);
    auto view = this->view(row, col);
    auto currentMousePos = view->mapToScene(event->pos());

    auto [hMovement, vMovement] = currentMousePos - m_prevMousePos;

    QVector3D movement;
    movement = hMovement * haxis + vMovement * vaxis;

    LOG_INFO(QString("offset changed: (%1, %2, %3)")
                 .arg(movement.x())
                 .arg(movement.y())
                 .arg(movement.z()));
    emit offsetChanged(movement);

    m_prevMousePos = currentMousePos;

}

void ScoutWidget::onViewWheeled(int row, int col, QWheelEvent *event) {
    const double rate = 0.01;
    auto delta = event->angleDelta().y() * rate;
    /// @todo 这里应该根据原来的角度，进行旋转，得到新的角度再返回
}
