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

    void setScoutFov(double other);

signals:
    void offsetChanged(QVector3D movement); /// 返回变化的量，单位是现实长度单位，与offset一致
    void angleChanged(QVector3D movement); /// 返回角度变化量

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QVector3D> m_angles;
    QList<QVector3D> m_offsets;
    double m_scoutFov;
    int m_rowNum=3;
    int m_colNum=3;

    /// row, col -> axis(0:x,1:y,2:y)
    QVector<std::pair<QVector3D, QVector3D>> m_axesMap;
    QPointF m_prevMousePos;

    /**
     *  @param scout扫描时传入的的角度
     *  @return [haxis, vaxis]，在图中向右和向下分别代表的空间方向
     *  @note 当谱仪的初始平面改动时，这个函数需要进行调整
     *  @todo 初始值写在setting中
     */
    std::pair<QVector3D, QVector3D> getViewAxes(QVector3D angle);

    /**
     * @brief 预览单个切片，改方法不会清除视图中已存在的内容
     * @param fov 所要添加的切片的fov
     * @param angles 切片的角度
     * @param offsets 切片的偏移
     */
    void previewSlice(double fov, QVector3D angles, QVector3D offsets);

    void onViewMousePressd(int row, int col, QMouseEvent *event);
    void onViewMouseMoved(int row, int col, QMouseEvent *event);
    void onViewWheeled(int row, int col, QWheelEvent *event);

    /// @todo 这些是由谱仪决定的常量，将来应该调整到配置里面，使其可修改
    constexpr const static QVector3D INIT_NORMAL_VECTOR = QVector3D(0, 0, 1);
    constexpr const static QVector3D INIT_HORIZONTAL_VECTOR = QVector3D(-1, 0, 0);
    constexpr const static QVector3D INIT_VERTICAL_VECTOR = QVector3D(0, -1, 0);
};

#endif // SCOUTWIDGET_H
