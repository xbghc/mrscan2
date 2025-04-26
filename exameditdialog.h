#ifndef EXAMEDITDIALOG_H
#define EXAMEDITDIALOG_H

#include <QDoubleSpinBox>
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>
#include <memory>

namespace Ui {
class ExamInfoDialog;
}

class ExamEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExamEditDialog(QWidget *parent = nullptr);
    ~ExamEditDialog();
    void setData(const QJsonObject& exam);
    QJsonObject getParameters();
    void setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles, QList<QVector3D> offsets);

private slots:
    void on_comboSlice_currentIndexChanged(int index);

    void on_checkGroupMode_stateChanged(int arg1);
private:
    QMap<QDoubleSpinBox*, QString> sliceSpinBoxKeyMap;
    QMap<QDoubleSpinBox*, int> sliceSpinBoxIndexMap;
    QMap<QAbstractSpinBox*, QString> paramEditKeyMap;

    std::unique_ptr<Ui::ExamInfoDialog> ui;
    struct SliceData{
        double m_data[6];

        double& operator[](int index) {
            assert(index >= 0 && index < 6);
            return m_data[index];
        }
        const double& operator[](int index)const {
            assert(index >= 0 && index < 6);
            return m_data[index];
        }
    };

    QVector<SliceData> m_slices;

    void setSlices(QJsonArray _slices);
    QJsonArray getSlices();
    void setSliceComboNumbers(int num);

    void initScoutWidget();

    void preview();
};

#endif // EXAMEDITDIALOG_H
