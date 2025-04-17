#ifndef EXAMMODEL_H
#define EXAMMODEL_H

#include "examitem.h"

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QSharedPointer>
#include <QMutex>
#include <QTime>
#include <QThread>

class ExamModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // 列枚举，使代码更清晰
    enum Column {
        NameColumn = 0,
        TimeColumn = 1,
        StatusColumn = 2,
        ColumnCount
    };
    
    explicit ExamModel(QObject *parent = nullptr);
    ~ExamModel();
    
    // QAbstractTableModel实现
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // 行操作方法
    void swapRows(int row1, int row2);
    void copyRow(int row);
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    
    // 检查数据访问
    QJsonObject getExamData(int row);
    void setExamParams(int row, const QJsonObject& parameters);
    void setExamResponse(int row, const QJsonObject& response);
    
    // 扫描流程管理
    void examStarted(int row, int id);
    int getScanningRow();
    int getScanningId();
    int examStoped();
    int examDone();
    bool isScanning() const { return m_scanningRow >= 0; }
    
    // 从JSON文件加载检查
    bool loadExams(const QString& filePath = "./configs/exams.json");
    bool saveExams(const QString& filePath = "./configs/exams.json") const;
    
signals:
    void examStatusChanged(int row, ExamItem::Status status);
    void timerUpdated(int row, const QString& time);
    
private slots:
    void updateTimers();
    
private:
    // 列标题
    QStringList m_headers;
    
    // 检查数据列表
    QList<ExamItem> m_exams;
    
    // 正在扫描的行和状态管理
    int m_scanningRow;
    void resetScanningState();
    
    // 计时相关
    QThread* m_timerThread;
    QTime m_startTime;
    QMutex m_mutex;
    bool m_threadShouldExit;
    
    // 帮助方法
    bool isValidRow(int row) const;
    void startTimerThread();
    void stopTimerThread();
    
    // 扫描完成/停止的通用方法
    int finishExam(int row, ExamItem::Status newStatus, bool shouldSave);
    
    // 从文件加载检查
    QList<ExamItem> loadExamsFromFile(const QString& filePath) const;
};

#endif // EXAMMODEL_H
