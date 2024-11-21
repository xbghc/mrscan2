#ifndef EXAMINFODIALOG_H
#define EXAMINFODIALOG_H

#include <QDoubleSpinBox>
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>

namespace Ui {
class ExamInfoDialog;
}

class ExamInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExamInfoDialog(QWidget *parent = nullptr);
    ~ExamInfoDialog();
    void setData(const QJsonObject& exam);
    QJsonObject getParameters();

private slots:
    void on_comboSlice_currentIndexChanged(int index);

    void on_checkGroupMode_stateChanged(int arg1);
private:
    QMap<QDoubleSpinBox*, QString> sliceSpinBoxKeyMap;
    QMap<QDoubleSpinBox*, int> sliceSpinBoxIndexMap;
    QMap<QAbstractSpinBox*, QString> paramEditKeyMap;

    Ui::ExamInfoDialog *ui;
    QVector<QVector<double>> m_slices;

    void setSlices(QJsonArray _slices);
    QJsonArray getSlices();
};

#endif // EXAMINFODIALOG_H
