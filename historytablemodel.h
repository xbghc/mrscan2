#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QList>

class HistoryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit HistoryTableModel(QObject *parent = nullptr);
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    void loadHistoryList();

    QStringList m_headers;
    struct HistoryItem{
        int examId;
        int patientId;
        QDateTime createTime;
    };
    QVector<HistoryItem> m_historyList;
};

#endif // HISTORYTABLEMODEL_H
