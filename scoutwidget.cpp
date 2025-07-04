#include "scoutwidget.h"
#include "configmanager.h"
#include "utils.h"
#include <QGraphicsPixmapItem>
#include <QMatrix4x4>
#include <QPen>

namespace {
const QString CNAME_AXIS_VECTOR = "axis_vector";
const QString KEY_INIT_NORMAL_VECTOR = "InitNormalVector";
const QString KEY_INIT_HORIZONTAL_VECTOR = "InitHorizontalVector";
const QString KEY_INIT_VERTICAL_VECTOR = "InitVerticalVector";

const QVector3D DEFAULT_INIT_NORMAL_VECTOR = QVector3D(0, 0, 1);
const QVector3D DEFAULT_INIT_HORIZONTAL_VECTOR = QVector3D(-1, 0, 0);
const QVector3D DEFAULT_INIT_VERTICAL_VECTOR = QVector3D(0, -1, 0);

void setNormalVector(const QVector3D &vec) {
    auto cm = ConfigManager::instance();
    QJsonObject obj;
    obj["x"] = vec.x();
    obj["y"] = vec.y();
    obj["z"] = vec.z();
    cm->set(CNAME_AXIS_VECTOR, KEY_INIT_NORMAL_VECTOR, obj);
}

QVector3D normalVector() {
    auto cm = ConfigManager::instance();
    auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_NORMAL_VECTOR);
    if (vec.isObject()) {
        auto obj = vec.toObject();
        return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(),
                         obj["z"].toDouble());
    }
    setNormalVector(DEFAULT_INIT_NORMAL_VECTOR);
    return DEFAULT_INIT_NORMAL_VECTOR;
}

void setHorizontalVector(const QVector3D &vec) {
    auto cm = ConfigManager::instance();
    QJsonObject obj;
    obj["x"] = vec.x();
    obj["y"] = vec.y();
    obj["z"] = vec.z();
    cm->set(CNAME_AXIS_VECTOR, KEY_INIT_HORIZONTAL_VECTOR, obj);
}

QVector3D horizontalVector() {
    auto cm = ConfigManager::instance();
    auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_HORIZONTAL_VECTOR);
    if (vec.isObject()) {
        auto obj = vec.toObject();
        return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(),
                         obj["z"].toDouble());
    }
    setHorizontalVector(DEFAULT_INIT_HORIZONTAL_VECTOR);
    return DEFAULT_INIT_HORIZONTAL_VECTOR;
}

void setVerticalVector(const QVector3D &vec) {
    auto cm = ConfigManager::instance();
    QJsonObject obj;
    obj["x"] = vec.x();
    obj["y"] = vec.y();
    obj["z"] = vec.z();
    cm->set(CNAME_AXIS_VECTOR, KEY_INIT_VERTICAL_VECTOR, obj);
}

QVector3D verticalVector() {
    auto cm = ConfigManager::instance();
    auto vec = cm->get(CNAME_AXIS_VECTOR, KEY_INIT_VERTICAL_VECTOR);
    if (vec.isObject()) {
        auto obj = vec.toObject();
        return QVector3D(obj["x"].toDouble(), obj["y"].toDouble(),
                         obj["z"].toDouble());
    }
    setVerticalVector(DEFAULT_INIT_VERTICAL_VECTOR);
    return DEFAULT_INIT_VERTICAL_VECTOR;
}
} // namespace

ScoutWidget::ScoutWidget(QWidget *parent) : QWidget(parent) {
    setupLayout();
    setupPlaneWidgets();
}

void ScoutWidget::setupLayout() {
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(2);
    m_grid->setContentsMargins(0, 0, 0, 0);
    setLayout(m_grid);
}

void ScoutWidget::setupPlaneWidgets() {
    m_transversePlaneWidget = new PlaneWidget(this);
    m_sagittalPlaneWidget = new PlaneWidget(this);
    m_coronalPlaneWidget = new PlaneWidget(this);

    m_grid->addWidget(m_transversePlaneWidget, 0, 0);
    m_grid->addWidget(m_sagittalPlaneWidget, 0, 1);
    m_grid->addWidget(m_coronalPlaneWidget, 0, 2);
}

void ScoutWidget::setScouts(QList<QImage> images, double fov,
                            QList<QVector3D> angles, QList<QVector3D> offsets) {
    m_transversePlaneWidget->setScoutFov(fov);
    m_sagittalPlaneWidget->setScoutFov(fov);
    m_coronalPlaneWidget->setScoutFov(fov);

    for (int i = 0; i < angles.length(); i++) {
        auto scout = std::make_shared<ScoutData>();
        scout->image = images[i];
        scout->angle = angles[i];
        scout->offset = offsets[i];

        // 使用getAxis函数判断属于哪个轴
        geometry_utils::Axis axis =
            geometry_utils::getAxis(angles[i], normalVector());
        switch (axis) {
        case geometry_utils::Axis::X:
            m_transversePlaneWidget->addScout(scout);
            break;
        case geometry_utils::Axis::Y:
            m_sagittalPlaneWidget->addScout(scout);
            break;
        case geometry_utils::Axis::Z:
            m_coronalPlaneWidget->addScout(scout);
            break;
        }
    }
}

void ScoutWidget::setSeparation(double separation) {
    m_transversePlaneWidget->setSeparation(separation);
    m_sagittalPlaneWidget->setSeparation(separation);
    m_coronalPlaneWidget->setSeparation(separation);
}

void ScoutWidget::setNoSlices(int noSlices) {
    m_transversePlaneWidget->setNoSlices(noSlices);
    m_sagittalPlaneWidget->setNoSlices(noSlices);
    m_coronalPlaneWidget->setNoSlices(noSlices);
}

void ScoutWidget::setSlices(QVector<std::shared_ptr<SliceData>> slices) {
    m_transversePlaneWidget->setSlices(slices);
    m_sagittalPlaneWidget->setSlices(slices);
    m_coronalPlaneWidget->setSlices(slices);
}

void ScoutWidget::setSlicesFov(double fov) {
    m_transversePlaneWidget->setFov(fov);
    m_sagittalPlaneWidget->setFov(fov);
    m_coronalPlaneWidget->setFov(fov);
}

void ScoutWidget::updateMarkers() {
    m_transversePlaneWidget->updateMarkers();
    m_sagittalPlaneWidget->updateMarkers();
    m_coronalPlaneWidget->updateMarkers();
}

// PlaneWidget 实现
void PlaneWidget::updateMarkers() {
    if (m_scoutDatas.empty()) {
        return;
    }

    updateView();
    updateLabel();
    updateButtons();
}

void PlaneWidget::setScoutFov(double fov) { m_scoutFov = fov; }

void PlaneWidget::drawCurrentScout() {
    auto image = currentScout()->image;
    image =
        image.scaled(static_cast<int>(m_scoutFov), static_cast<int>(m_scoutFov),
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    auto pixmap = m_view->scene()->addPixmap(QPixmap::fromImage(image));
    pixmap->setPos(-m_scoutFov / 2, -m_scoutFov / 2);
}

void PlaneWidget::drawSlice(SliceData *slice) {
    auto scout = currentScout();
    auto scoutAngle = scout->angle;
    auto scoutOffset = scout->offset;

    auto [point, vector] = geometry_utils::computePlaneIntersection(
        scoutAngle, scoutOffset, slice->angle(), slice->offset(),
        normalVector());

    if (vector.lengthSquared() < 1e-6) {
        return;
    }

    auto pointOnViewX = QVector3D::dotProduct(point, m_hDirection);
    auto pointOnViewY = QVector3D::dotProduct(point, m_vDirection);

    auto vectorOnViewX = QVector3D::dotProduct(vector, m_hDirection);
    auto vectorOnViewY = QVector3D::dotProduct(vector, m_vDirection);

    auto lineEdge = m_fov;
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

    QPen pen(Qt::red);
    pen.setWidth(1);
    line->setPen(pen);
    m_view->scene()->addItem(line);
}

void PlaneWidget::drawSlices() {
    if (m_slices.empty()) {
        return;
    }

    if (m_separation == 0) {
        for (auto& slice : m_slices) {
            drawSlice(slice.get());
        }
    } else {
        // m_slices长度为1，手动生成其他图片
        auto centralAngle = m_slices[0]->angle();
        auto centralOffset = m_slices[0]->offset();
        for (int i = 0; i < m_noSlices; i++) {
            auto offset = centralOffset;
            auto vector = geometry_utils::applyRotate(centralAngle, normalVector());
            offset = offset + vector * (i - m_noSlices / 2) * m_separation;
            SliceData slice{centralAngle, offset};
            drawSlice(&slice);
        }
    }
}

PlaneWidget::PlaneWidget(QWidget *parent)
    : QWidget(parent), m_fov(0.0), m_currentIndex(0) {
    setupUI();
    setupConnections();
}

bool PlaneWidget::eventFilter(QObject *watched, QEvent *event) {
    auto viewport = qobject_cast<QWidget *>(watched);
    if (!viewport || m_view->viewport() != viewport) {
        return QWidget::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        onViewMousePressd(static_cast<QMouseEvent *>(event));
        return false;
    case QEvent::MouseMove:
        onViewMouseMoved(static_cast<QMouseEvent *>(event));
        return false;
    case QEvent::Wheel:
        onViewWheeled(static_cast<QWheelEvent *>(event));
        return false;
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

void PlaneWidget::onViewMousePressd(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }

    m_prevMousePos = m_view->mapToScene(event->pos());
}

void PlaneWidget::onViewMouseMoved(QMouseEvent *event) {
    auto currentMousePos = m_view->mapToScene(event->pos());
    auto [hMovement, vMovement] = currentMousePos - m_prevMousePos;
    auto movement = hMovement * m_hDirection + vMovement * m_vDirection;

    for (auto& slice : m_slices) {
        slice->setOffset(slice->offset() + movement);
    }

    m_prevMousePos = currentMousePos;
    updateMarkers();
}

void PlaneWidget::onViewWheeled(QWheelEvent *event) {
    auto delta = event->angleDelta().y() * 0.01;
    auto movement = delta * m_normalDirection;

    for (auto& slice : m_slices) {
        auto oldMatrix = geometry_utils::rotateMatrix(slice->angle());
        auto newMatrix = oldMatrix * geometry_utils::rotateMatrix(movement);
        // 由矩阵计算角度(x,y,z), 要求旋转顺序为xyz
        QQuaternion quat =
            QQuaternion::fromRotationMatrix(newMatrix.toGenericMatrix<3, 3>());
        auto newAngle = quat.toEulerAngles();
        slice->setAngle(newAngle);
    }

    updateMarkers();
}

void PlaneWidget::setFov(double fov) { m_fov = fov; }

void PlaneWidget::setSlices(QVector<std::shared_ptr<SliceData>> slices) {
    // 清空之前连接
    for (auto& slice : m_slices) {
        disconnect(slice.get(), &SliceData::angleChanged, this, nullptr);
        disconnect(slice.get(), &SliceData::offsetChanged, this, nullptr);
    }

    m_slices.clear();
    for (auto& slice : slices) {
        m_slices.append(slice);
        connect(slice.get(), &SliceData::angleChanged, this,
                [this](QVector3D angle) {
                    updateMarkers();
                });
        connect(slice.get(), &SliceData::offsetChanged, this,
                [this](QVector3D offset) {
                    updateMarkers();
                });
    }

    updateMarkers();
}

void PlaneWidget::addScout(std::shared_ptr<ScoutData> scout) {
    if (m_scoutDatas.empty()) {
        m_angle = scout->angle;

        m_normalDirection = geometry_utils::applyRotate(m_angle, normalVector());
        m_hDirection = geometry_utils::applyRotate(m_angle, horizontalVector());
        m_vDirection = geometry_utils::applyRotate(m_angle, verticalVector());
    } else if ((m_angle - scout->angle).lengthSquared() > 1e-6) {
        LOG_ERROR("Scout angle is not consistent");
    }

    m_scoutDatas.append(scout);

    updateMarkers();
}

void PlaneWidget::setSeparation(double separation) {
    m_separation = separation;
    updateMarkers();
}

void PlaneWidget::setNoSlices(int noSlices) {
     m_noSlices = noSlices;
     updateMarkers();
}

ScoutData *PlaneWidget::currentScout() const {
    return m_scoutDatas[m_currentIndex].get();
}

void PlaneWidget::setupUI() {
    m_layout = new QVBoxLayout(this);
    m_view = new QGraphicsView(this);
    m_label = new QLabel(this);
    m_buttonWidget = new QWidget(this);
    m_prevButton = new QPushButton("◀", m_buttonWidget);
    m_nextButton = new QPushButton("▶", m_buttonWidget);

    // setup view
    auto scene = new QGraphicsScene(m_view);
    m_view->setScene(scene);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setAlignment(Qt::AlignCenter);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // setup label
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet(
        "QLabel { color: #666; font-size: 12px; padding: 2px; }");

    // setup buttons
    m_prevButton->setFixedSize(30, 25);
    m_nextButton->setFixedSize(30, 25);
    m_prevButton->setStyleSheet("QPushButton { font-size: 12px; }");
    m_nextButton->setStyleSheet("QPushButton { font-size: 12px; }");

    // setup buttonWidget
    auto buttonWidgetLayout = new QHBoxLayout(m_buttonWidget);
    buttonWidgetLayout->setContentsMargins(2, 2, 2, 2);
    buttonWidgetLayout->setSpacing(5);
    buttonWidgetLayout->addStretch();
    buttonWidgetLayout->addWidget(m_prevButton);
    buttonWidgetLayout->addWidget(m_nextButton);
    buttonWidgetLayout->addStretch();

    // setup layout
    m_layout->addWidget(m_view);
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_buttonWidget);
}

void PlaneWidget::setupConnections() {
    connect(m_prevButton, &QPushButton::clicked, this,
            &PlaneWidget::onPrevButtonClicked);
    connect(m_nextButton, &QPushButton::clicked, this,
            &PlaneWidget::onNextButtonClicked);

    m_view->viewport()->installEventFilter(this);
}

void PlaneWidget::updateLabel() {
    QString text = QString("Offset:(%1, %2, %3)")
                       .arg(m_scoutDatas[m_currentIndex]->offset.x(), 0, 'f', 2)
                       .arg(m_scoutDatas[m_currentIndex]->offset.y(), 0, 'f', 2)
                       .arg(m_scoutDatas[m_currentIndex]->offset.z(), 0, 'f', 2);
    m_label->setText(text);
}

void PlaneWidget::updateButtons() {
    m_prevButton->setEnabled(m_currentIndex > 0);
    m_nextButton->setEnabled(m_currentIndex < m_scoutDatas.size() - 1);
}

void PlaneWidget::updateView() {
    m_view->scene()->clear();

    m_view->setSceneRect(-m_fov / 2, -m_fov / 2, m_fov, m_fov);

    drawCurrentScout();
    drawSlices();
}

QVector3D PlaneWidget::angle() const { return m_angle; }

void PlaneWidget::onPrevButtonClicked() {
    m_currentIndex = (m_currentIndex - 1 + m_scoutDatas.size()) % m_scoutDatas.size();
    updateMarkers();
}

void PlaneWidget::onNextButtonClicked() {
    m_currentIndex = (m_currentIndex + 1) % m_scoutDatas.size();
    updateMarkers();
}
