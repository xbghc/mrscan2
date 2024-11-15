#ifndef EXAMINFODIALOG_H
#define EXAMINFODIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>

namespace Ui {
class ExamInfoDialog;
}

class ExamInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExamInfoDialog(QWidget *parent = nullptr);
    ~ExamInfoDialog();
    void setData(QJsonObject& exam);
    QJsonObject getParameters();

private slots:
    void on_comboSlice_currentIndexChanged(int index);

    void on_checkGroupMode_stateChanged(int arg1);

    void on_buttonBox_accepted();

    void on_editXAngle_valueChanged(double arg1);

    void on_editYAngle_valueChanged(double arg1);

    void on_editZAngle_valueChanged(double arg1);

    void on_editXOffset_valueChanged(double arg1);

    void on_editYOffset_valueChanged(double arg1);

    void on_editZOffset_valueChanged(double arg1);

private:
    Ui::ExamInfoDialog *ui;
    QJsonArray slices;
    void setSlices(QJsonArray _slices);
    bool validate();
};

#endif // EXAMINFODIALOG_H
