#ifndef EXAMTAB_H
#define EXAMTAB_H

#include <QJsonObject>
#include <QModelIndex>
#include <QWidget>
#include "exammodel.h"
#include "examhistory.h"
#include <memory>

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
    void loadPatients();
    void updateScanButtonState(bool isScanning);
    void enablePatientSelection(bool enable);
    QString getStatus(int row);
    int getCurrentPatientId() const;
    QJsonObject getCurrentExam() const;

public slots:
    void onScanStarted(int id);
    void onScanEnd(QByteArray response);

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

signals:
    void onStartButtonClicked(QJsonObject sequence);
    void onStopButtonClicked(int id);
    void fileSaved(ExamHistory history);

private:
    Ui::studytab *ui;
    std::unique_ptr<ExamModel> examModel;
};

#endif // EXAMTAB_H
