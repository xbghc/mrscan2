#include "historymodel.h"

#include <QDir>
#include <QJsonDocument>
#include "pathmanager.h"
#include "utils.h"

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

void HistoryModel::loadHistoryList()
{
    beginResetModel();
    m_historyList.clear();

    QStringList patientIds = PathManager::getAllPatientIds();
    for(const auto& patientIdStr : patientIds) {
        int patientId = patientIdStr.toInt();
        QStringList examIds = PathManager::getExamIdsForPatient(patientId);
        
        for(const auto &examIdStr : examIds) {
            int examId = examIdStr.toInt();
            QString examDirPath = PathManager::getExamDir(patientId, examId);
            QFileInfo dirInfo(examDirPath);
            QDateTime createTime = dirInfo.birthTime();
            m_historyList.append(HistoryItem(examId, patientId, createTime));
        }
    }
    endResetModel();
}
