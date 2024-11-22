#include "historytablemodel.h"

#include <QDir>
#include <QJsonDocument>

HistoryTableModel::HistoryTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers << "scan id" << "patient id" << "scan datetime";
    loadHistoryList();
}

QVariant HistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section >= 0 && section < m_headers.size()) {
        return m_headers[section];
    }

    return QVariant();
}

int HistoryTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_historyList.count();
}

int HistoryTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_headers.count();
}

QVariant HistoryTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (!index.isValid())
        return QVariant();

    int row = index.row();
    const auto& item = m_historyList[row];
    switch (index.column()) {
    case 0:
        return item.examId;
        break;
    case 1:
        return item.patientId;
    case 2:
        return item.createTime;
    default:
        break;
    }
    return QVariant();
}

void HistoryTableModel::loadHistoryList()
{
    beginResetModel();
    m_historyList.clear();

    QDir rootDir("./patients");
    rootDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    for(const auto& patientId:rootDir.entryList()){
        QDir patientDir(rootDir.filePath(patientId));
        if(!patientDir.exists()){
            qDebug() << "unexpected directory structure";
            continue;
        }

        patientDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        for(const auto &examId:patientDir.entryList()){
            QString examDirPath = patientDir.filePath(examId);
            QFileInfo dirInfo(examDirPath);
            QDateTime createTime = dirInfo.birthTime();
            m_historyList.append(HistoryItem(examId.toInt(), patientId.toInt(), createTime));
        }
    }
    endResetModel();
}

ExamHistory HistoryTableModel::getHistoryObj(int row)
{
    if(row < 0 || row > m_historyList.count()){
        qDebug() << "index out of history list";
        return ExamHistory();
    }

    int patientId = m_historyList[row].patientId;
    int examId = m_historyList[row].examId;
    return ExamHistory(patientId, examId);
}
