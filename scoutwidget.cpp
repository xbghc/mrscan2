#include "scoutwidget.h"
#include <QMatrix4x4>
#include <QPen>
#include <QGraphicsPixmapItem>
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

ScoutWidget::ScoutWidget(QWidget *parent) : QWidget(parent) {
    m_initNormalVector = normalVector();
    m_initHorizontalVector = horizontalVector();
    m_initVerticalVector = verticalVector();

    setupLayout();
}

void ScoutWidget::setupLayout()
{
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(2);
    m_grid->setContentsMargins(0, 0, 0, 0);
    setLayout(m_grid);
}

void ScoutWidget::setupGrid()
{
    const QString axisNames[COL_NUM] = {"X-Axis", "Y-Axis", "Z-Axis"};
    
    // 设置所有轴视图
    for (int col = 0; col < COL_NUM; ++col) {
        auto& axisView = m_axisViews[col];
        axisView.setup(axisNames[col], this);
        axisView.view->viewport()->installEventFilter(this);
        
        m_grid->addWidget(axisView.view, 0, col);
        m_grid->addWidget(axisView.label, 1, col);
        m_grid->addWidget(axisView.buttonWidget, 2, col);
    }

    // 设置行拉伸比例
    m_grid->setRowStretch(0, 10);  // 视图行
    m_grid->setRowStretch(1, 1);   // 标签行
    m_grid->setRowStretch(2, 1);   // 按钮行

    // 连接按钮信号
    connect(m_axisViews[0].prevButton, &QPushButton::clicked, this, &ScoutWidget::onXPrevClicked);
    connect(m_axisViews[0].nextButton, &QPushButton::clicked, this, &ScoutWidget::onXNextClicked);
    connect(m_axisViews[1].prevButton, &QPushButton::clicked, this, &ScoutWidget::onYPrevClicked);
    connect(m_axisViews[1].nextButton, &QPushButton::clicked, this, &ScoutWidget::onYNextClicked);
    connect(m_axisViews[2].prevButton, &QPushButton::clicked, this, &ScoutWidget::onZPrevClicked);
    connect(m_axisViews[2].nextButton, &QPushButton::clicked, this, &ScoutWidget::onZNextClicked);
}

bool ScoutWidget::addLine(int index, QGraphicsLineItem* line)
{
    if (index < 0 || index >= COL_NUM) {
        return false;
    }
    
    QGraphicsView* targetView = qobject_cast<QGraphicsView*>(m_grid->itemAtPosition(0, index)->widget());
    if (targetView && targetView->scene()) {
        targetView->scene()->addItem(line);
        return true;
    }
    return false;
}

std::pair<int, int> ScoutWidget::viewPortPosition(QWidget* viewport) const {
    if (!viewport) {
        return {-1, -1};
    }

    for (int col = 0; col < COL_NUM; ++col) {
        if (m_axisViews[col].view && m_axisViews[col].view->viewport() == viewport) {
            return {0, col};
        }
    }

    return {-1, -1};
}

const QGraphicsView* ScoutWidget::view(int row, int col) const {
    if (row != 0 || col < 0 || col >= COL_NUM) {
        return nullptr;
    }
    return m_axisViews[col].view;
}

/**
 * @brief 计算单个scout slice的场景偏移
 * @param scoutSlice 包含图像、角度和偏移信息的scout slice
 * @return QPair<double, double> 返回水平和垂直方向的场景偏移
 * @details 
 * 该函数基于scout slice的3D角度和偏移信息，计算在2D视图中的场景偏移。
 * 计算步骤：
 * 1. 根据scout的角度获取视图的水平轴和垂直轴方向
 * 2. 将3D偏移投影到视图的水平轴和垂直轴上
 * 3. 减去FOV的一半，使图像居中显示
 */
QPair<double, double> ScoutWidget::calculateSceneOffset(const ScoutSlice& scoutSlice) {
    // 获取当前scout角度对应的视图坐标轴
    auto [hAxis, vAxis] = getViewAxes(scoutSlice.angle);
    
    // 将3D偏移投影到2D视图坐标系
    double hOffset = QVector3D::dotProduct(scoutSlice.offset, hAxis) - m_scoutFov / 2;
    double vOffset = QVector3D::dotProduct(scoutSlice.offset, vAxis) - m_scoutFov / 2;
    
    return qMakePair(hOffset, vOffset);
}

/**
 * @brief 根据角度判断属于哪个轴
 * @param angle 旋转角度
 * @return 轴枚举：Axis::X, Axis::Y, Axis::Z
 * @details 
 * 通过将初始法向量按给定角度旋转，然后判断旋转后的向量主要沿哪个坐标轴方向
 */
Axis ScoutWidget::getAxis(const QVector3D& angle) const {
    // 将初始法向量按给定角度旋转
    QVector3D rotatedNormal = rotateMatrix(angle).map(m_initNormalVector);
    
    // 取绝对值找到最大的分量
    double absX = std::abs(rotatedNormal.x());
    double absY = std::abs(rotatedNormal.y());
    double absZ = std::abs(rotatedNormal.z());
    
    // 返回最大分量对应的轴
    if (absX >= absY && absX >= absZ) {
        return Axis::X;
    } else if (absY >= absZ) {
        return Axis::Y;
    } else {
        return Axis::Z;
    }
}

void ScoutWidget::setScoutImages(QList<QImage> images, double fov,
                                 QList<QVector3D> angles,
                                 QList<QVector3D> offsets) {
    m_scoutFov = fov;
    
    // 重置所有轴数据
    for (int i = 0; i < AXIS_COUNT; ++i) {
        m_axisData[i].reset();
    }
    
    // 根据角度判断坐标轴方向，将scout slice分类到对应的数组
    for (int i = 0; i < angles.length(); i++) {
        ScoutSlice slice;
        slice.image = images[i];
        slice.angle = angles[i];
        slice.offset = offsets[i];
        
        // 使用getAxis函数判断属于哪个轴
        Axis axis = getAxis(angles[i]);
        m_axisData[static_cast<int>(axis)].slices.append(slice);
    }
    
    // 清除preview缓存
    m_hasPreviewData = false;
    m_previewSlices.clear();
    
    setupGrid();
    updateMarkers();
}

void ScoutWidget::updateMarkers() {
    if (m_scoutFov == 0) {
        return;
    }

    // 更新所有轴视图
    for (int col = 0; col < COL_NUM; ++col) {
        auto axisView = view(0, col);
        if (!axisView || !axisView->scene()) continue;
        
        const auto& axisData = m_axisData[col];
        const ScoutSlice* currentSlice = axisData.getCurrentSlice();
        
        auto scene = axisView->scene();
        scene->clear();
        
        if (currentSlice) {
            auto offset = calculateSceneOffset(*currentSlice);
            scene->setSceneRect(offset.first, offset.second, m_scoutFov, m_scoutFov);
            
            auto image = currentSlice->image;
            image = image.scaled(static_cast<int>(m_scoutFov), static_cast<int>(m_scoutFov), 
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            auto pixmap = scene->addPixmap(QPixmap::fromImage(image));
            pixmap->setPos(offset.first, offset.second);
        }
    }
    
    updateAllDisplays();
    
    // 重绘缓存的preview线条
    if (m_hasPreviewData) {
        redrawPreviewLines();
    }
}

void ScoutWidget::updateAllDisplays() {
    for (int col = 0; col < COL_NUM; ++col) {
        auto& axisView = m_axisViews[col];
        const auto& axisData = m_axisData[col];
        
        if (axisData.hasData()) {
            const auto* currentSlice = axisData.getCurrentSlice();
            if (currentSlice) {
                axisView.updateDisplay(axisData.currentIndex, axisData.slices.size(), currentSlice->offset);
            } else {
                axisView.setNoData();
            }
        } else {
            axisView.setNoData();
        }
        
        axisView.updateButtons(axisData.hasMultipleSlices());
    }
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
    if (!hasAnyAxisData()) {
        return;
    }

    QVector<QPair<QVector3D, QVector3D>> slices;
    auto v = rotateMatrix(angles).map(m_initNormalVector);
    for (int i = 0; i < noSlices; i++) {
        double o = (i - (static_cast<double>(noSlices) - 1) / 2) * sliceSeparation;
        auto offset = o * v + offsets;
        slices.append(qMakePair(angles, offset));
    }

    preview(fov, thickness, slices);
}

void ScoutWidget::preview(double fov, double thickness,
                          QVector<QPair<QVector3D, QVector3D>> slices) {
    if (!hasAnyAxisData()) {
        return;
    }

    // 缓存preview参数
    m_previewFov = fov;
    m_previewThickness = thickness;
    m_previewSlices = slices;
    m_hasPreviewData = true;

    updateMarkers();
}

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

    // 遍历所有当前显示的scout slices
    for (int col = 0; col < COL_NUM; ++col) {
        ScoutSlice* currentSlice = getCurrentSlice(static_cast<Axis>(col));
        if (!currentSlice) continue;
        
        auto scoutAngle = currentSlice->angle;
        auto scoutOffset = currentSlice->offset;

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
        addLine(col, line);
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
    ScoutSlice* currentSlice = getCurrentSlice(static_cast<Axis>(col));
    if (!currentSlice) return;
    
    auto [haxis, vaxis] = getViewAxes(currentSlice->angle);
    auto view = this->view(row, col);
    auto currentMousePos = view->mapToScene(event->pos());

    auto [hMovement, vMovement] = currentMousePos - m_prevMousePos;
    QVector3D movement = hMovement * haxis + vMovement * vaxis;

    emit offsetChanged(movement);
    m_prevMousePos = currentMousePos;
}

void ScoutWidget::onViewWheeled(int row, int col, QWheelEvent *event) {
    ScoutSlice* currentSlice = getCurrentSlice(static_cast<Axis>(col));
    if (!currentSlice) return;
    
    const double rate = 0.01;
    auto delta = event->angleDelta().y() * rate;
    auto axis = rotateMatrix(currentSlice->angle).map(m_initNormalVector);

    emit angleChanged(axis * delta);
}

void ScoutWidget::onXPrevClicked() { switchSlice(Axis::X, false); }
void ScoutWidget::onXNextClicked() { switchSlice(Axis::X, true); }
void ScoutWidget::onYPrevClicked() { switchSlice(Axis::Y, false); }
void ScoutWidget::onYNextClicked() { switchSlice(Axis::Y, true); }
void ScoutWidget::onZPrevClicked() { switchSlice(Axis::Z, false); }
void ScoutWidget::onZNextClicked() { switchSlice(Axis::Z, true); }

void ScoutWidget::switchSlice(Axis axis, bool next) {
    auto& axisData = getAxisData(axis);
    
    if (!axisData.hasMultipleSlices()) return;
    
    if (next) {
        axisData.currentIndex = (axisData.currentIndex + 1) % axisData.slices.size();
    } else {
        axisData.currentIndex = (axisData.currentIndex - 1 + axisData.slices.size()) % axisData.slices.size();
    }
    
    updateMarkers();
}

ScoutSlice* ScoutWidget::getCurrentSlice(Axis axis) {
    return getAxisData(axis).getCurrentSlice();
}

bool ScoutWidget::hasAnyAxisData() const {
    for (int i = 0; i < AXIS_COUNT; ++i) {
        if (m_axisData[i].hasData()) {
            return true;
        }
    }
    return false;
}

void ScoutWidget::redrawPreviewLines() {
    if (!m_hasPreviewData || m_previewSlices.isEmpty()) {
        return;
    }
    
    // 为每个缓存的slice绘制preview线条
    for (const auto& slice : m_previewSlices) {
        previewSlice(m_previewFov, slice.first, slice.second);
    }
}

void ScoutWidget::clearPreview() {
    // 清除缓存
    m_hasPreviewData = false;
    m_previewSlices.clear();
    
    // 重新绘制markers（不包含preview线条）
    updateMarkers();
}



// AxisView 实现
void ScoutWidget::AxisView::setup(const QString& axisName, QWidget* parent) {
    m_axisName = axisName;
    
    // 创建视图
    view = new QGraphicsView(parent);
    QGraphicsScene* scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setAlignment(Qt::AlignCenter);
    view->setFrameShape(QFrame::NoFrame);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 创建标签
    label = new QLabel(axisName, parent);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { color: #666; font-size: 12px; padding: 2px; }");
    
    // 创建按钮
    buttonWidget = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(5);
    
    prevButton = new QPushButton("◀", parent);
    nextButton = new QPushButton("▶", parent);
    prevButton->setFixedSize(30, 25);
    nextButton->setFixedSize(30, 25);
    prevButton->setStyleSheet("QPushButton { font-size: 12px; }");
    nextButton->setStyleSheet("QPushButton { font-size: 12px; }");
    
    layout->addStretch();
    layout->addWidget(prevButton);
    layout->addWidget(nextButton);
    layout->addStretch();
}

void ScoutWidget::AxisView::updateDisplay(int currentIndex, int totalCount, const QVector3D& offset) {
    if (!label) return;
    
    QString text = QString("%1 [%2/%3] Offset:(%4, %5, %6)")
                  .arg(m_axisName)
                  .arg(currentIndex + 1)
                  .arg(totalCount)
                  .arg(offset.x(), 0, 'f', 2)
                  .arg(offset.y(), 0, 'f', 2)
                  .arg(offset.z(), 0, 'f', 2);
    label->setText(text);
}

void ScoutWidget::AxisView::updateButtons(bool hasMultipleSlices) {
    if (prevButton && nextButton) {
        prevButton->setEnabled(hasMultipleSlices);
        nextButton->setEnabled(hasMultipleSlices);
    }
}

void ScoutWidget::AxisView::setNoData() {
    if (label) {
        label->setText(m_axisName + " (No Data)");
    }
}
