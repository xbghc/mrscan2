#include "scoutwidget.h"
#include "utils.h"
#include <QMatrix4x4>
#include <QPen>

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
    m_scoutSlices.clear();
    for (int i = 0; i < angles.length(); i++) {
        m_scoutSlices.append(qMakePair(angles[i], offsets[i]));
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
    if (m_scoutSlices.empty()) {
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

void ScoutWidget::preview(double fov, double thickness,
                          QVector<QPair<QVector3D, QVector3D>> slices) {
    if (m_scoutSlices.empty()) {
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
    auto n1 = QVector3D(A1, B1, C1);
    auto n2 = QVector3D(A2, B2, C2);

    QVector3D line_direction = QVector3D::crossProduct(n1, n2);
    constexpr double epsilon = 1e-9; // 根据精度需求调整

    // 1. 检查平面是否平行或重合
    if (line_direction.lengthSquared() <
        epsilon * epsilon) { // 使用平方避免开方，epsilon也要平方比较
        // 法向量平行，平面可能平行或重合
        // 如果需要区分平行且不重合（无交线）与重合（无限交线），需要进一步检查：
        // 例如，检查一个平面上的点是否在另一个平面上。
        // 或者检查 (A1,B1,C1,D1) 和 (A2,B2,C2,D2) 是否线性相关。
        // 对于此函数，如果目标是返回一条“唯一”交线，则平行或重合都算作无唯一交线。
        return {QVector3D(), QVector3D()}; // 表示无唯一交线或错误
    }

    QVector3D point_on_line;
    // 使用平面方程 Ax + By + Cz = -D 的形式
    double d1_val = -D1;
    double d2_val = -D2;

    // 2. 计算交线上的一点 (更鲁棒的方法)
    // 通过选择 line_direction 中绝对值最大的分量来决定将哪个坐标设为0，
    // 以确保求解时使用的行列式（即 line_direction
    // 的对应分量）不为零且较大，提高数值稳定性。
    double absLx = std::abs(line_direction.x());
    double absLy = std::abs(line_direction.y());
    double absLz = std::abs(line_direction.z());

    if (absLx >= absLy && absLx >= absLz) { // line_direction.x() 绝对值最大或之一
        // 设 x = 0. 求解:
        // B1*y + C1*z = d1_val
        // B2*y + C2*z = d2_val
        // det = B1*C2 - B2*C1 = line_direction.x()
        double det = line_direction.x();
        point_on_line.setX(0);
        point_on_line.setY((d1_val * C2 - d2_val * C1) / det);
        point_on_line.setZ((B1 * d2_val - B2 * d1_val) / det);
    } else if (absLy >= absLx &&
               absLy >= absLz) { // line_direction.y() 绝对值最大或之一
        // 设 y = 0. 求解:
        // A1*x + C1*z = d1_val
        // A2*x + C2*z = d2_val
        // det_sys = A1*C2 - A2*C1 = -line_direction.y()
        double det =
            -line_direction.y(); // (因为 line_direction.y() = A2*C1 - A1*C2)
        point_on_line.setY(0);
        point_on_line.setX((d1_val * C2 - d2_val * C1) / det);
        point_on_line.setZ((A1 * d2_val - A2 * d1_val) / det);
    } else { // line_direction.z() 绝对值最大
        // 设 z = 0. 求解:
        // A1*x + B1*y = d1_val
        // A2*x + B2*y = d2_val
        // det = A1*B2 - A2*B1 = line_direction.z()
        double det = line_direction.z();
        point_on_line.setZ(0);
        point_on_line.setX((d1_val * B2 - d2_val * B1) / det);
        point_on_line.setY((A1 * d2_val - A2 * d1_val) / det);
    }

    // 返回交线上一点和归一化的方向向量
    return {point_on_line, line_direction.normalized()};
}

QPair<QVector3D, QVector3D>
ScoutWidget::intersectionLine(const QVector3D angle1, const QVector3D offset1,
                              const QVector3D angle2,
                              const QVector3D offset2) const {
    auto v1 = rotateMatrix(angle1).map(INIT_NORMAL_VECTOR);
    auto v2 = rotateMatrix(angle2).map(INIT_NORMAL_VECTOR);
    auto D1 = -(QVector3D::dotProduct(v1, offset1));
    auto D2 = -(QVector3D::dotProduct(v2, offset2));
    return intersectionLine(v1.x(), v1.y(), v1.z(), D1, v2.x(), v2.y(), v2.z(),
                            D2);
}

void ScoutWidget::previewSlice(double fov, QVector3D angles,
                               QVector3D offsets) {
    /// @note 可选择是否将slice视为无边界的平面，只需要调整lineEdge

    LOG_INFO(QString("offset: %1, %2, %3")
                 .arg(offsets.x())
                 .arg(offsets.y())
                 .arg(offsets.z()));
    for (int i = 0; i < m_scoutSlices.length(); i++) {
        auto scoutAngle = m_scoutSlices[i].first;
        auto scoutOffset = m_scoutSlices[i].second;

        auto [point, vector] =
            intersectionLine(scoutAngle, scoutOffset, angles, offsets);

        if (vector.lengthSquared() < 1e-6) {
            continue;
        }

        // 计算与视图边缘的交点位置
        auto [hAxis, vAxis] = getViewAxes(scoutAngle);

        // point在视图中的投影
        auto pointOnViewX = QVector3D::dotProduct(point, hAxis);
        auto pointOnViewY = QVector3D::dotProduct(point, vAxis);

        // vector在视图中的投影
        auto vectorOnViewX = QVector3D::dotProduct(vector, hAxis);
        auto vectorOnViewY = QVector3D::dotProduct(vector, vAxis);

        auto lineEdge = m_scoutFov;
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        if (std::abs(vectorOnViewX) > std::abs(vectorOnViewY)) {
            x1 = -lineEdge / 2;
            x2 = lineEdge / 2;
            y1 = (x1 - pointOnViewX) * vectorOnViewY / vectorOnViewX + pointOnViewY;
            y2 = (x2 - pointOnViewX) * vectorOnViewY / vectorOnViewX + pointOnViewY;
        } else {
            y1 = -lineEdge / 2;
            y2 = lineEdge / 2;
            x1 = (y1 - pointOnViewY) * vectorOnViewX / vectorOnViewY + pointOnViewX;
            x2 = (y2 - pointOnViewY) * vectorOnViewX / vectorOnViewY + pointOnViewX;
        }
        auto line = new QGraphicsLineItem(x1, y1, x2, y2);

        QPen pen(Qt::red);          // Color
        pen.setWidth(3);            // Line width
        // pen.setStyle(Qt::DashLine); // Dashed line
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
    auto [haxis, vaxis] = getViewAxes(m_scoutSlices[row * m_colNum + col].first);
    auto view = this->view(row, col);
    auto currentMousePos = view->mapToScene(event->pos());

    auto [hMovement, vMovement] = currentMousePos - m_prevMousePos;

    QVector3D movement;
    movement = hMovement * haxis + vMovement * vaxis;

    emit offsetChanged(movement);

    m_prevMousePos = currentMousePos;
}

void ScoutWidget::onViewWheeled(int row, int col, QWheelEvent *event) {
    const double rate = 0.01;
    auto delta = event->angleDelta().y() * rate;
    /// @todo 这里应该根据原来的角度，进行旋转，得到新的角度再返回
}
