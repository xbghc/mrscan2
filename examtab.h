#ifndef EXAMTAB_H
#define EXAMTAB_H

#include "exam.h"
#include "patientinfodialog.h"
#include "exameditdialog.h"

#include <QJsonObject>
#include <QModelIndex>
#include <QWidget>
#include <QTimer>
#include <memory>

namespace Ui {
class examtab;
}

/**
 * @todo Remove dependency on JsonPatient, use unified IPatient interface
 * @todo Adjust patient storage structure
 * @brief The ExamTab class
 */
class ExamTab : public QWidget {
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // exam related
    int currentRow() const; /// Current exam row
    int processingRow() const;
    const Exam &currentExam() const;

    // patient related
    void updatePatientList(bool reload = false);
    IPatient* getPatient(QString id);
    QString currentPatientId() const;

    /**
     * @brief Set the response of the current scan, meaning the scan is complete
     */
    const Exam &setResponse(IExamResponse *response);
public slots:
    void onScanStarted(QString id);
    void onScanStoped(); /// User manually stopped

private slots:
    void onEditPatientButtonClicked(); /// Button: Edit patient
    void onNewPatientButtonClicked();  /// Button: New patient
    void onPatientDialogAccepted();    /// Patient dialog confirmed

    void onDeletePatientButtonClicked(); /// Button: Delete patient
    void onShiftUpButtonClicked();       /// Button: ↑
    void onShiftDownButtonClicked();     /// Button: ↓
    void onRemoveExamButtonClicked();    /// Button: Remove sequence
    void onCopyExamButtonClicked();      /// Button: Copy sequence
    void onEditExamButtonClicked();      /// Button: Edit sequence
    void onScanStopButtonClicked();      /// Button: Start scan/Stop scan
    void onExamDialogAccept(); /// Exam dialog confirmed

    void onCurrentExamChanged();
signals:
    void startButtonClicked(ExamRequest exam);
    void stopButtonClicked(QString id);

private slots:
    void tick();  /// Timer during scanning process

private:
    std::unique_ptr<Ui::examtab> ui;
    QMap<QString, std::shared_ptr<IPatient>> m_patientMap;
    QVector<Exam> m_exams;
    QTimer m_timer;

    std::unique_ptr<PatientInfoDialog> m_patientDialog;
    std::unique_ptr<ExamEditDialog> m_examDialog;

    void setupConnections(); // Set up all signal connections
    void resizeTableToContents(); // Adjust table size to fit content
    QString generateNewPatientId(); // Generate new patient ID
    void addPatient(QString name, QDate birthday, IPatient::Gender gender);

    void swap(int row1, int row2);
    void updateExamTable();
};

#endif // EXAMTAB_H
