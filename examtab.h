#ifndef EXAMTAB_H
#define EXAMTAB_H

#include <QWidget>

#include "examhistory.h"
#include "examtablemodel.h"

namespace Ui {
class studytab;
}

class ExamTab : public QWidget
{
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();
    int currentExamIndex();

signals:
    void currentExamChanged(QJsonObject patient, QJsonObject exam);
    void fileSaved(ExamHistory history); // TODO 移除
    void onStartButtonClicked(QJsonObject& exam); // TODO 移除参数
    void onStopButtonClicked(int id); // TODO 移除参数

public slots:
    void loadPatients();
    void onScanStarted(int id);
    void onScanEnd(QByteArray responseBytes);

private slots:
    void openEditPatientDialog();

    void openNewPatientDialog();

    void deletePatient();

    void shiftUp();

    void shiftDown();

    void removeExam();

    void copyExam();

    void editExam();

    void onScanButtonClicked();

private:
    Ui::studytab *ui;
    ExamTableModel* examModel;

    QString getStatus(int row);
};

#endif // EXAMTAB_H
