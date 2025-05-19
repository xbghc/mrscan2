#ifndef EXAMTAB_H
#define EXAMTAB_H

#include "exam.h"
#include "patientinfodialog.h"
#include "exameditdialog.h"

#include <QJsonObject>
#include <QModelIndex>
#include <QWidget>
#include <memory>

namespace Ui {
class examtab;
}

/**
 * @todo 移除对JsonPatient的依赖，使用统一的IPatient接口
 * @todo 调整patient的存储结构
 * @brief The ExamTab class
 */
class ExamTab : public QWidget {
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();

    // exam related
    int currentRow() const; /// 当前exam所在行
    int processingRow() const;
    const Exam &currentExam() const;

    // patient related
    void updatePatientList(bool reload = false);
    IPatient* getPatient(QString id);
    QString currentPatientId() const;

    const Exam &setResponse(IExamResponse *response);
public slots:
    void onScanStarted(QString id);
    void onScanStoped(); /// 用户手动停止

private slots:
    void onEditPatientButtonClicked(); /// 按钮： 编辑病人
    void onNewPatientButtonClicked();  /// 按钮： 新建病人
    void onPatientDialogAccepted();    /// 病人窗口中点击确定

    void onDeletePatientButtonClicked(); /// 按钮：删除病人
    void onShiftUpButtonClicked();       /// 按钮： ↑
    void onShiftDownButtonClicked();     /// 按钮： ↓
    void onRemoveExamButtonClicked();    /// 按钮： 删除序列
    void onCopyExamButtonClicked();      /// 按钮： 复制序列
    void onEditExamButtonClicked();      /// 按钮： 编辑序列
    void onScanStopButtonClicked();      /// 按钮： 开始扫描/停止扫描
    void onExamDialogAccept(); /// Exam窗口点击确定

    void onCurrentExamChanged();
signals:
    void startButtonClicked(ExamRequest exam);
    void stopButtonClicked(QString id);

private:
    std::unique_ptr<Ui::examtab> ui;

    QList<Exam> m_exams;
    QVector<std::shared_ptr<IPatient>> m_patients;
    std::unique_ptr<PatientInfoDialog> m_patientDialog;

    std::unique_ptr<ExamEditDialog> m_examDialog;

    /// @note
    /// 如果是正规dicom格式的病人数据，不至于在核磁软件中创建病人，所以这里放个生成新id的函数就够了
    /// @todo
    /// 调整id生成的逻辑。uitls中添加生成随机数的函数，随机生成再+1遍历没有被使用的id作为新id

    void addPatient(QString name, QDate birthday, IPatient::Gender gender);

    int nextPatientId();
    void setNextId(int id);

    void swap(int row1, int row2);
    void updateExamTable();
};

#endif // EXAMTAB_H
