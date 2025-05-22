#include "historymodel.h"

#include <QDir>
#include <QJsonDocument>
#include "store.h"

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers << "scan id" << "patient id" << "scan datetime";
    loadHistoryList();
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section >= 0 && section < m_headers.size()) {
        return m_headers[section];
    }

    return QVariant();
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_historyList.count();
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_headers.count();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
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

void HistoryModel::addExam(const QString &examId, const QString &patientId, const QDateTime &createTime)
{
    int row = 0;
    beginInsertRows(QModelIndex(), row, row);
    m_historyList.insert(row, {examId, patientId, createTime});
    endInsertRows();
}

void HistoryModel::loadHistoryList()
{
    auto map = store::examMap();

    beginResetModel();

    m_historyList.clear();

    for(const auto& [pid, eids]:map){
        for(const auto& eid:eids){
            auto path = store::edir(pid, eid);
            auto btime = QFileInfo(path).birthTime();
            m_historyList.emplace_back(eid, pid, btime);
        }
    }

    endResetModel();
}
