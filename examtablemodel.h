#ifndef EXAMTABLEMODEL_H
#define EXAMTABLEMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>

class ExamTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ExamTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void swapRows(int row1, int row2);
    void copyRow(int row);
    void removeRow(int row);

    QJsonObject getExamData(int row);
    void setExamParams(int row, QJsonObject _exam);
private:
    QStringList m_headers;
    QJsonArray m_data;
    QList<QTime> m_time;
    QList<int> m_status;
};

#endif // EXAMTABLEMODEL_H
