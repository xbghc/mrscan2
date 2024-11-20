#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QJsonObject>
#include <QList>

class HistoryTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static void saveExam(const unsigned char*header, const unsigned char*data, QJsonObject examObj, int patientId);
    static QJsonObject loadExamInfo(int patientId, int examId);
    static QVariant loadExamData(int patientId, int examId, QJsonObject examObj);

    explicit HistoryTableModel(QObject *parent = nullptr);
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    static QString getExamDirPath(int patientId, int examId);
    static QString getResponseFilePath(int patientId, int examId);
    static QString getRequestFilePath(int patientId, int examId);

    QStringList m_headers;
    QList<QJsonObject> m_examHistoryList;
};

#endif // HISTORYTABLEMODEL_H
