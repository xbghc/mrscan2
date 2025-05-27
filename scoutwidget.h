#ifndef SCOUTWIDGET_H
#define SCOUTWIDGET_H

#include <QJsonObject>
#include <QMouseEvent>
#include <QVector3D>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>

struct ScoutSlice {
    QImage image;
    QVector3D angle;
    QVector3D offset;
};

enum class Axis : int {
    X = 0,
    Y = 1,
    Z = 2
};

class ScoutWidget : public QWidget {
    Q_OBJECT

private:
    /**
     * @brief 轴视图内部类，封装单个轴的所有UI组件
     * @details 每个AxisView实例管理一个轴（X/Y/Z）的完整UI，包括：
     *          - 图像显示视图
     *          - 信息标签（显示轴名、索引、偏移）
     *          - 切换按钮（上一个/下一个slice）
     */
    class AxisView {
    public:
        // UI组件
        QGraphicsView* view;        ///< 图像显示视图
        QLabel* label;              ///< 信息标签
        QPushButton* prevButton;    ///< 上一个slice按钮
        QPushButton* nextButton;    ///< 下一个slice按钮
        QWidget* buttonWidget;      ///< 按钮容器
        
        AxisView() : view(nullptr), label(nullptr), prevButton(nullptr), 
                    nextButton(nullptr), buttonWidget(nullptr) {}
        
        /**
         * @brief 初始化轴视图的所有UI组件
         * @param axisName 轴名称（如"X-Axis"）
         * @param parent 父窗口
         */
        void setup(const QString& axisName, QWidget* parent);
        
        /**
         * @brief 更新显示信息
         * @param currentIndex 当前slice索引
         * @param totalCount 总slice数量
         * @param offset 3D偏移坐标
         */
        void updateDisplay(int currentIndex, int totalCount, const QVector3D& offset);
        
        /**
         * @brief 更新按钮启用状态
         * @param hasMultipleSlices 是否有多个slice可切换
         */
        void updateButtons(bool hasMultipleSlices);
        
        /**
         * @brief 设置无数据状态显示
         */
        void setNoData();
        
    private:
        QString m_axisName;  ///< 轴名称
    };

public:
    ScoutWidget(QWidget *parent = nullptr);

    void setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles,
                        QList<QVector3D> offsets);
    void updateMarkers();

    void preview(double fov, double thickness, double sliceSeparation,
                 int noSlices, QVector3D angles, QVector3D offsets);
    void preview(double fov, double thickness,
                 QVector<QPair<QVector3D, QVector3D>> slices);
    void clearPreview();  ///< 清除preview缓存和显示

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

    // 添加从QImagesWidget继承的必要方法
    const QGraphicsView* view(int row, int col) const;
    bool addLine(int index, QGraphicsLineItem* line);
    std::pair<int, int> viewPortPosition(QWidget* viewport) const;

signals:
    void offsetChanged(
        QVector3D movement); /// Return the amount of change, unit is real length unit, consistent with offset
    void angleChanged(QVector3D movement); /// Return angle change amount

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void onViewMousePressd(int row, int col, QMouseEvent *event);
    void onViewMouseMoved(int row, int col, QMouseEvent *event);
    void onViewWheeled(int row, int col, QWheelEvent *event);

private slots:
    void onXPrevClicked();
    void onXNextClicked();
    void onYPrevClicked();
    void onYNextClicked();
    void onZPrevClicked();
    void onZNextClicked();

private:
    static constexpr int ROW_NUM = 1;
    static constexpr int COL_NUM = 3;
    static constexpr int AXIS_COUNT = 3;
    
    // 轴数据结构
    struct AxisData {
        QVector<ScoutSlice> slices;
        int currentIndex = 0;
        
        void reset() {
            slices.clear();
            currentIndex = 0;
        }
        
        bool hasData() const { return !slices.isEmpty(); }
        bool hasMultipleSlices() const { return slices.size() > 1; }
        const ScoutSlice* getCurrentSlice() const {
            return (currentIndex < slices.size()) ? &slices[currentIndex] : nullptr;
        }
        ScoutSlice* getCurrentSlice() {
            return (currentIndex < slices.size()) ? &slices[currentIndex] : nullptr;
        }
    };
    
    // 数据成员
    AxisData m_axisData[AXIS_COUNT];  ///< 三个轴的数据
    double m_scoutFov = 0.0;

    // 3D空间配置
    QVector3D m_initNormalVector;
    QVector3D m_initHorizontalVector;
    QVector3D m_initVerticalVector;

    // UI组件
    QGridLayout* m_grid;
    AxisView m_axisViews[COL_NUM];  // 使用数组简化管理
    
    // 鼠标交互
    QPointF m_prevMousePos;
    
    // Preview缓存 - 用于在updateMarkers时自动重绘preview线条
    bool m_hasPreviewData = false;                        ///< 是否有缓存的preview数据
    double m_previewFov = 0.0;                           ///< 缓存的FOV参数
    double m_previewThickness = 0.0;                     ///< 缓存的厚度参数
    QVector<QPair<QVector3D, QVector3D>> m_previewSlices; ///< 缓存的切片数据 (angles, offsets)
    
    // 私有方法
    void setupLayout();
    void setupGrid();
    void updateAllDisplays();
    QPair<double, double> calculateSceneOffset(const ScoutSlice& scoutSlice);
    Axis getAxis(const QVector3D& angle) const;
    AxisView& getAxisView(int col) { return m_axisViews[col]; }
    const AxisView& getAxisView(int col) const { return m_axisViews[col]; }
    void switchSlice(Axis axis, bool next);
    ScoutSlice* getCurrentSlice(Axis axis);
    AxisData& getAxisData(Axis axis) { return m_axisData[static_cast<int>(axis)]; }
    const AxisData& getAxisData(Axis axis) const { return m_axisData[static_cast<int>(axis)]; }
    bool hasAnyAxisData() const;  ///< 检查是否有任何轴的数据
    void redrawPreviewLines();  ///< 重绘缓存的preview线条

    /**
   * @brief Preview a single slice, this method will not clear existing content in the view
   * @param fov FOV of the slice to be added
   * @param angles Angle of the slice
   * @param offsets Offset of the slice
   */
    void previewSlice(double fov, QVector3D angles, QVector3D offsets);
};

#endif // SCOUTWIDGET_H
