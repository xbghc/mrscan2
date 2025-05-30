#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QList>

class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit HistoryModel(QObject *parent = nullptr);
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void loadHistoryList();
    void addExam(const QString& examId, const QString& patientId, const QDateTime& createTime);

private:
    QStringList m_headers;
    struct HistoryItem{
        QString examId;
        QString patientId;
        QDateTime createTime;
    };
    QVector<HistoryItem> m_historyList;

};

#endif // HISTORYMODEL_H
