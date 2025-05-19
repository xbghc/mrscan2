#ifndef EXAMEDITDIALOG_H
#define EXAMEDITDIALOG_H

#include "exam.h"

#include <QDialog>
#include <QDoubleSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QPair>
#include <QVector3D>
#include <QVector>
#include <memory>

namespace Ui {
class ExamInfoDialog;
}

class ExamEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExamEditDialog(QWidget *parent = nullptr);
    ~ExamEditDialog();
    void setData(const Exam &exam);
    QJsonObject getParameters();

    void setScout(const Exam &exam);

    /**
   * @brief 获取偏移量
   * @details 如果是组模式，返回的是中心偏移量，否则返回的是当前切片的偏移量
   * @return 偏移量
   */
    QVector3D offset() const;
    /**
   * @brief 设置偏移量
   * @details 如果是组模式，设置的是中心偏移量，否则设置的是当前切片的偏移量
   * @param other 偏移量
   */
    void setOffset(QVector3D other);

    QVector3D angle() const;
    void setAngle(QVector3D other);

    constexpr const static char *KEY_X_OFFSET = "xOffset";
    constexpr const static char *KEY_Y_OFFSET = "yOffset";
    constexpr const static char *KEY_Z_OFFSET = "zOffset";
    constexpr const static char *KEY_X_ANGLE = "xAngle";
    constexpr const static char *KEY_Y_ANGLE = "yAngle";
    constexpr const static char *KEY_Z_ANGLE = "zAngle";

    constexpr const static char *KEY_FOV = "fov";
    constexpr const static char *KEY_NO_AVERAGES = "noAverages";
    constexpr const static char *KEY_NO_SLICES = "noSlices";
    constexpr const static char *KEY_NO_SAMPLES = "noSamples";
    constexpr const static char *KEY_NO_VIEWS = "noViews";
    constexpr const static char *KEY_OBSERVE_FREQUENCY = "observeFrequency";
    constexpr const static char *KEY_SLICE_THICKNESS = "sliceThickness";
    constexpr const static char *KEY_SLICE_SEPARATION = "sliceSeparation";
    constexpr const static char *KEY_SLICES = "slices";

private slots:
    void on_comboSlice_currentIndexChanged(int index);

private:
    std::unique_ptr<Ui::ExamInfoDialog> ui;

    /**
   * @brief 切片信息
   * @details 第一个QVector3D是角度，第二个QVector3D是偏移量,
   * 如果为组模式，则m_slices长度为1，否则为noSlices
   */
    QVector<QPair<QVector3D, QVector3D>> m_slices;

    void setSlices(QJsonArray _slices);
    QJsonArray jsonSlices();
    void setSliceComboNumbers(int num);

    bool shouldRepaint = true;

    void preview();

    void resisterEditerSignals();
};

#endif // EXAMEDITDIALOG_H
