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
    // Column enumeration to make code more readable
    enum Column {
        NameColumn = 0,
        TimeColumn = 1,
        StatusColumn = 2,
        ColumnCount
    };
    
    explicit ExamModel(QObject *parent = nullptr);
    ~ExamModel();
    
    // QAbstractTableModel implementation
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // Row operation methods
    void swapRows(int row1, int row2);
    void copyRow(int row);
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    
    // Exam data access
    QJsonObject getExamData(int row);
    void setExamParams(int row, const QJsonObject& parameters);
    void setExamResponse(int row, const QJsonObject& response);
    
    // Scan process management
    void examStarted(int row, int id);
    int getScanningRow();
    int getScanningId();
    int examStoped();
    int examDone();
    bool isScanning() const { return m_scanningRow >= 0; }
    
    // Load exams from JSON file
    bool loadExams(const QString& filePath = "./configs/exams.json");
    bool saveExams(const QString& filePath = "./configs/exams.json") const;
    
signals:
    void examStatusChanged(int row, ExamItem::Status status);
    void timerUpdated(int row, const QString& time);
    
private slots:
    void updateTimers();
    
private:
    // Column headers
    QStringList m_headers;
    
    // Exam data list
    QList<ExamItem> m_exams;
    
    // Scanning row and status management
    int m_scanningRow;
    void resetScanningState();
    
    // Timer related
    QThread* m_timerThread;
    QTime m_startTime;
    QMutex m_mutex;
    bool m_threadShouldExit;
    
    // Helper methods
    bool isValidRow(int row) const;
    void startTimerThread();
    void stopTimerThread();
    
    // Common method for scan completion/stop
    int finishExam(int row, ExamItem::Status newStatus, bool shouldSave);
    
    // Load exams from file
    QList<ExamItem> loadExamsFromFile(const QString& filePath) const;
};

#endif // EXAMMODEL_H
