#ifndef EXAMMODEL_H
#define EXAMMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QThread>

class ExamModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class ExamStatus {
        Ready = 0,
        Processing = 1,
        Done = 2,
    };

    explicit ExamModel(QObject *parent = nullptr);
    ~ExamModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void swapRows(int row1, int row2);
    void copyRow(int row);
    void removeRow(int row);

    QJsonObject getExamData(int row);
    void setExamParams(int row, QJsonObject parameters);
    void setExamResponse(int row, QJsonObject response);
    void examStarted(int row, int id);
    int getScanningRow();
    int getScanningId();
    int examStoped();
    int examDone();
private:
    QStringList m_headers;
    QJsonArray m_data;
    QList<QString> m_time;
    QList<ExamStatus> m_status;
    QThread* timerThread;
};

#endif // EXAMMODEL_H
