#include "scoutwidget.h"
#include <QMatrix4x4>
#include <QPen>
#include "configmanager.h"
#include "utils.h"

namespace {
    const QString CNAME_AXIS_VECTOR = "axis_vector";
    const QString KEY_INIT_NORMAL_VECTOR = "InitNormalVector";
    const QString KEY_INIT_HORIZONTAL_VECTOR = "InitHorizontalVector";
    const QString KEY_INIT_VERTICAL_VECTOR = "InitVerticalVector";

    const QVector3D DEFAULT_INIT_NORMAL_VECTOR = QVector3D(0, 0, 1);
    const QVector3D DEFAULT_INIT_HORIZONTAL_VECTOR = QVector3D(-1, 0, 0);
    const QVector3D DEFAULT_INIT_VERTICAL_VECTOR = QVector3D(0, -1, 0);

    void setNormalVector(const QVector3D& vec) {
        auto cm = ConfigManager::instance();
        QJsonObject obj;
        obj["x"] = vec.x();
        obj["y"] = vec.y();
        obj["z"] = vec.z();
        cm->set(CNAME_AXIS_VECTOR, KEY_INIT_NORMAL_VECTOR, obj);
    }

    QVector3D normalVector(){
        auto cm = ConfigManager::instance();
        auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_NORMAL_VECTOR);
        if (vec.isObject()) {
            auto obj = vec.toObject();
            return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(), obj["z"].toDouble());
        }
        setNormalVector(DEFAULT_INIT_NORMAL_VECTOR);
        return DEFAULT_INIT_NORMAL_VECTOR;
    }

    void setHorizontalVector(const QVector3D& vec) {
        auto cm = ConfigManager::instance();
        QJsonObject obj;
        obj["x"] = vec.x();
        obj["y"] = vec.y();
        obj["z"] = vec.z();
        cm->set(CNAME_AXIS_VECTOR, KEY_INIT_HORIZONTAL_VECTOR, obj);
    }

    QVector3D horizontalVector(){
        auto cm = ConfigManager::instance();
        auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_HORIZONTAL_VECTOR);
        if (vec.isObject()) {
            auto obj = vec.toObject();
            return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(), obj["z"].toDouble());
        }
        setHorizontalVector(DEFAULT_INIT_HORIZONTAL_VECTOR);
        return DEFAULT_INIT_HORIZONTAL_VECTOR;
    }

    void setVerticalVector(const QVector3D& vec) {
        auto cm = ConfigManager::instance();
        QJsonObject obj;
        obj["x"] = vec.x();
        obj["y"] = vec.y();
        obj["z"] = vec.z();
        cm->set(CNAME_AXIS_VECTOR, KEY_INIT_VERTICAL_VECTOR, obj);
    }

    QVector3D verticalVector(){
        auto cm = ConfigManager::instance();
        auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_VERTICAL_VECTOR);
        if (vec.isObject()) {
            auto obj = vec.toObject();
            return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(), obj["z"].toDouble());
        }
        setVerticalVector(DEFAULT_INIT_VERTICAL_VECTOR);
        return DEFAULT_INIT_VERTICAL_VECTOR;
    }
}

ScoutWidget::ScoutWidget(QWidget *parent) : QImagesWidget(parent) {
    m_initNormalVector = normalVector();
    m_initHorizontalVector = horizontalVector();
    m_initVerticalVector = verticalVector();

    setRowNum(m_rowNum);
    setColNum(m_colNum);
    layout()->setSpacing(20);
    setImages(QList<QImage>());
}

void ScoutWidget::setScoutImages(QList<QImage> images, double fov,
                                 QList<QVector3D> angles,
                                 QList<QVector3D> offsets) {
    QImagesWidget::setImages(images);
    QImagesWidget::setSceneWidth(fov);
    QImagesWidget::setSceneHeight(fov);

    m_scoutFov = fov;
    m_scoutSlices.clear();
    
    for (int i = 0; i < angles.length(); i++) {
        ScoutSlice slice;
        slice.image = images[i];
        slice.angle = angles[i];
        slice.offset = offsets[i];
        m_scoutSlices.append(slice);
    }

    for (int r = 0; r < m_rowNum; r++) {
        for (int c = 0; c < m_colNum; c++) {
            auto [hAxis, vAxis] = getViewAxes(m_scoutSlices[r * m_colNum + c].angle);
            auto offset = m_scoutSlices[r * m_colNum + c].offset;
            setSceneOffset(r, c, QVector3D::dotProduct(offset, hAxis) - m_scoutFov / 2, QVector3D::dotProduct(offset, vAxis) - m_scoutFov / 2);
        }
    }
}

void ScoutWidget::updateMarkers() {
    QImagesWidget::updateMarkers();
}

std::pair<QVector3D, QVector3D>
ScoutWidget::getViewAxes(QVector3D angle) const {
    auto hAxis = m_initHorizontalVector;
    auto vAxis = m_initVerticalVector;

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

    QVector<ScoutSlice> slices;
    auto v = rotateMatrix(angles).map(m_initNormalVector);
    for (int i = 0; i < noSlices; i++) {
        double o = (i - (static_cast<double>(noSlices) - 1) / 2) * sliceSeparation;
        auto offset = o * v + offsets;
        ScoutSlice slice;
        slice.angle = angles;
        slice.offset = offset;
        slices.append(slice);
    }

    preview(fov, thickness, slices);
}

void ScoutWidget::preview(double fov, double thickness,
                          QVector<ScoutSlice> slices) {
    if (m_scoutSlices.empty()) {
        return;
    }

    updateMarkers();

    for (int i = 0; i < slices.length(); i++) {
        previewSlice(fov, slices[i].angle, slices[i].offset);
    }
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
    constexpr double epsilon = 1e-9; // Adjust based on precision requirements

    // 1. Check if planes are parallel or coincident
    if (line_direction.lengthSquared() <
        epsilon * epsilon) { // Use square to avoid sqrt, epsilon also needs to be squared for comparison
        // Normal vectors are parallel, planes may be parallel or coincident
        // To distinguish between parallel but not coincident (no intersection line) and coincident (infinite intersection lines), further checks are needed:
        // For example, check if a point on one plane is on the other plane.
        // Or check if (A1,B1,C1,D1) and (A2,B2,C2,D2) are linearly related.
        // For this function, if the goal is to return a "unique" intersection line, both parallel and coincident cases are considered as no unique intersection line.
        return {QVector3D(), QVector3D()}; // Indicates no unique intersection line or error
    }

    QVector3D point_on_line;
    // Use plane equation in the form Ax + By + Cz = -D
    double d1_val = -D1;
    double d2_val = -D2;

    // 2. Calculate a point on the intersection line (more robust method)
    // By selecting the component with the largest absolute value in line_direction to decide which coordinate to set to 0,
    // to ensure that the determinant used in solving (i.e., the corresponding component of line_direction)
    // is not zero and is large, improving numerical stability.
    double absLx = std::abs(line_direction.x());
    double absLy = std::abs(line_direction.y());
    double absLz = std::abs(line_direction.z());

    if (absLx >= absLy && absLx >= absLz) { // line_direction.x() has the largest absolute value or one of them
        // Set x = 0. Solve:
        // B1*y + C1*z = d1_val
        // B2*y + C2*z = d2_val
        // det = B1*C2 - B2*C1 = line_direction.x()
        double det = line_direction.x();
        point_on_line.setX(0);
        point_on_line.setY((d1_val * C2 - d2_val * C1) / det);
        point_on_line.setZ((B1 * d2_val - B2 * d1_val) / det);
    } else if (absLy >= absLx &&
               absLy >= absLz) { // line_direction.y() has the largest absolute value or one of them
        // Set y = 0. Solve:
        // A1*x + C1*z = d1_val
        // A2*x + C2*z = d2_val
        // det_sys = A1*C2 - A2*C1 = -line_direction.y()
        double det =
            -line_direction.y(); // (because line_direction.y() = A2*C1 - A1*C2)
        point_on_line.setY(0);
        point_on_line.setX((d1_val * C2 - d2_val * C1) / det);
        point_on_line.setZ((A1 * d2_val - A2 * d1_val) / det);
    } else { // line_direction.z() has the largest absolute value
        // Set z = 0. Solve:
        // A1*x + B1*y = d1_val
        // A2*x + B2*y = d2_val
        // det = A1*B2 - A2*B1 = line_direction.z()
        double det = line_direction.z();
        point_on_line.setZ(0);
        point_on_line.setX((d1_val * B2 - d2_val * B1) / det);
        point_on_line.setY((A1 * d2_val - A2 * d1_val) / det);
    }

    // Return a point on the intersection line and the normalized direction vector
    return {point_on_line, line_direction.normalized()};
}

QPair<QVector3D, QVector3D>
ScoutWidget::intersectionLine(const QVector3D angle1, const QVector3D offset1,
                              const QVector3D angle2,
                              const QVector3D offset2) const {
    auto v1 = rotateMatrix(angle1).map(m_initNormalVector);
    auto v2 = rotateMatrix(angle2).map(m_initNormalVector);
    auto D1 = -(QVector3D::dotProduct(v1, offset1));
    auto D2 = -(QVector3D::dotProduct(v2, offset2));
    return intersectionLine(v1.x(), v1.y(), v1.z(), D1, v2.x(), v2.y(), v2.z(),
                            D2);
}

void ScoutWidget::previewSlice(double fov, QVector3D angles,
                               QVector3D offsets) {
    /// @note Can choose whether to treat slice as an unbounded plane, just need to adjust lineEdge

    for (int i = 0; i < m_scoutSlices.length(); i++) {
        auto scoutAngle = m_scoutSlices[i].angle;
        auto scoutOffset = m_scoutSlices[i].offset;

        auto [point, vector] =
            intersectionLine(scoutAngle, scoutOffset, angles, offsets);

        if (vector.lengthSquared() < 1e-6) {
            continue;
        }

        // Calculate intersection positions with view edges
        auto [hAxis, vAxis] = getViewAxes(scoutAngle);

        // Projection of point in the view
        auto pointOnViewX = QVector3D::dotProduct(point, hAxis);
        auto pointOnViewY = QVector3D::dotProduct(point, vAxis);

        // Projection of vector in the view
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

        QPen pen(Qt::red); // Color
        pen.setWidth(3);   // Line width
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
    auto [haxis, vaxis] = getViewAxes(m_scoutSlices[row * m_colNum + col].angle);
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

    auto axis = rotateMatrix(m_scoutSlices[row * m_colNum + col].angle)
                    .map(m_initNormalVector);

    emit angleChanged(axis * delta);
}
