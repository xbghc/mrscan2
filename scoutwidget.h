#ifndef SCOUTWIDGET_H
#define SCOUTWIDGET_H

#include "geometry_utils.h"
#include <QEvent>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWheelEvent>
#include <QWidget>
#include <memory>

struct ScoutData {
    QImage image;
    QVector3D angle;
    QVector3D offset;
};

class SliceData : public QObject {
    Q_OBJECT
public:
    SliceData(QVector3D angle, QVector3D offset)
        : m_angle(angle), m_offset(offset) {}

    QVector3D angle() const { return m_angle; }
    void setAngle(QVector3D angle) {
        if (m_angle == angle) {
            return;
        }

        m_angle = angle;
        emit angleChanged(angle);
    }

    QVector3D offset() const { return m_offset; }
    void setOffset(QVector3D offset) {
        if (m_offset == offset) {
            return;
        }

        m_offset = offset;
        emit offsetChanged(offset);
    }

private:
    QVector3D m_angle;
    QVector3D m_offset;

signals:
    void offsetChanged(QVector3D offset);
    void angleChanged(QVector3D angle);
};

/**
 * @brief 轴视图内部类，封装单个轴的所有UI组件
 * @details 每个PlaneWidget实例管理一个轴（X/Y/Z）的完整UI，包括：
 *          - 图像显示视图
 *          - 信息标签（显示轴名、索引、偏移）
 *          - 切换按钮（上一个/下一个slice）
 */
class PlaneWidget : public QWidget {
    Q_OBJECT
public:
    PlaneWidget(QWidget *parent = nullptr);

    QVector3D angle() const;
    void setSlices(QVector<std::shared_ptr<SliceData>> slices);
    void addScout(std::shared_ptr<ScoutData> scout);

    void updateMarkers();
    void updateLabel();
    void updateButtons();
    void updateView();

    void setFov(double fov); // set slice fov
    void setScoutFov(double fov);
    ScoutData *currentScout() const;

    void setSeparation(double separation);
    void setNoSlices(int noSlices);

signals:
    void offsetChanged(QVector3D offset);
    void angleChanged(QVector3D angle);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

protected slots:
    void onPrevButtonClicked();
    void onNextButtonClicked();

    void onViewMousePressd(QMouseEvent *event);
    void onViewMouseMoved(QMouseEvent *event);
    void onViewWheeled(QWheelEvent *event);

private:
    QVBoxLayout *m_layout;
    QGraphicsView *m_view;     ///< 图像显示视图
    QLabel *m_label;           ///< 信息标签
    QPushButton *m_prevButton; ///< 上一个slice按钮
    QPushButton *m_nextButton; ///< 下一个slice按钮
    QWidget *m_buttonWidget;   ///< 按钮容器

    double m_fov;      ///< slice fov
    double m_scoutFov; ///< scout fov

    QVector<std::shared_ptr<ScoutData>> m_scoutDatas; ///< 所有scout数据
    QVector<std::shared_ptr<SliceData>> m_slices;     ///< 要预览的slice数据
    int m_currentIndex;                              ///< 当前scout索引

    /** 
     * @brief 相邻slice之间的距离
     * @details 当separation为0时，说明只有一张图片
     */
    double m_separation;
    int m_noSlices;  ///< 仅当separation不为0时有效

    // cache data
    QVector3D m_normalDirection;
    QVector3D m_hDirection;
    QVector3D m_vDirection;
    QVector3D m_angle;  ///< scout的角度

    QPointF m_prevMousePos;

    void drawSlice(SliceData *slice);
    void drawSlices();
    void drawCurrentScout();

    void setupUI();
    void setupConnections();
};

class ScoutWidget : public QWidget {
    Q_OBJECT

public:
    ScoutWidget(QWidget *parent = nullptr);

    void setScouts(QList<QImage> images, double fov, QList<QVector3D> angles,
                   QList<QVector3D> offsets);

    void setSlices(QVector<std::shared_ptr<SliceData>> slices);

    void setSlicesFov(double fov);

    void setSeparation(double separation);
    void setNoSlices(int noSlices);

    /**
     * @brief 主动触发内部视图的更新
     */
    void updateMarkers();

private:
    // UI组件
    QGridLayout *m_grid;
    PlaneWidget *m_transversePlaneWidget;
    PlaneWidget *m_sagittalPlaneWidget;
    PlaneWidget *m_coronalPlaneWidget;

    // 初始化函数
    void setupLayout();
    void setupPlaneWidgets();
};

#endif // SCOUTWIDGET_H
