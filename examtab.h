#ifndef EXAMTAB_H
#define EXAMTAB_H

#include <QJsonObject>
#include <QModelIndex>
#include <QWidget>
#include "exam.h"
#include "patientinfodialog.h"
#include <memory>

namespace Ui {
class examtab;
}

/**
 * @todo 移除对JsonPatient的依赖，使用统一的IPatient接口
 * @todo 调整patient的存储结构
 * @brief The ExamTab class
 */
class ExamTab : public QWidget
{
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();

    // exam related
    int currentRow() const;
    int processingRow() const;
    const Exam& currentExam() const;

    // patient related
    void loadPatients();
    void updateScanButtonState(bool isScanning);
    void enablePatientSelection(bool enable);
    QString currentPatientId() const;

    JsonPatient getPatient(QString id);
    const Exam& onResponseReceived(IExamResponse* response);
public slots:
    void onScanStarted(QString id);

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
    void startButtonClicked(ExamRequest exam);
    void stopButtonClicked(QString id);

private:
    std::unique_ptr<Ui::examtab> ui;

    QList<Exam> m_exams;
    QVector<JsonPatient> m_patients; /// @todo 应该使用IPatient接口

    void addPatient(QString name, QDate birthday, IPatient::Gender gender);
    void savePatients();
    void removePatient(QString id);
    /// @note 如果是正规dicom格式的病人数据，不至于在核磁软件中创建病人，所以这里放个生成新id的函数就够了
    int nextPatientId();
    void setNextId(int id);

    std::unique_ptr<PatientInfoDialog> m_patientDialog;

    void swap(int row1, int row2);
    void updateExamTable();
};

#endif // EXAMTAB_H
