#include "examtablemodel.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace{
QJsonArray loadExams(){
    const static QString kPath = "./configs/exams.json";

    QDir dir("./configs");
    if (!dir.exists() && dir.mkpath(".")) {
        qDebug() << "failed to mkdir: " << dir.path();
        return QJsonArray();
    }

    QFile file(kPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qDebug() << "Warning: No Exam Configuration!";
        return QJsonArray();
    }

    QJsonArray exams = QJsonDocument::fromJson(file.readAll()).array();
    return exams;
}
}

ExamTableModel::ExamTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{
    m_data = loadExams();
    for(int i=0;i<m_data.size();i++){
        m_time.append(QTime());
        m_status.append(0);
    }

    m_headers << "Sequence" << "Time" << "Status";
}

int ExamTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int ExamTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_headers.size();
}

QVariant ExamTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int row = index.row();
    int col = index.column();

    const static QStringList kStatus{"Ready", "Processing", "Done"};

    switch (col) {
    case 0:
        return m_data[row].toObject()["name"].toString();
    case 1:
        return m_time[row];
    case 2:
        return kStatus[m_status[row]];
    default:
        return QVariant();
    }
}

QVariant ExamTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_headers[section];
    }
    return QVariant();
}

void ExamTableModel::swapRows(int row1, int row2)
{
    if (row1 < 0 || row1 >= m_data.size() || row2 < 0 || row2 >= m_data.size() || row1 == row2)
        return;

    beginMoveRows(QModelIndex(), row1, row1, QModelIndex(), row2 > row1 ? row2 + 1 : row2);

    QJsonValue temp1 = m_data[row1];
    QJsonValue temp2 = m_data[row2];
    m_data.replace(row1, temp2);
    m_data.replace(row2, temp1);

    m_time.swapItemsAt(row1, row2);
    m_status.swapItemsAt(row1, row2);
    endMoveRows();
}

void ExamTableModel::copyRow(int row)
{
    if (row < 0 || row >= m_data.size())
        return;

    beginInsertRows(QModelIndex(), row + 1, row + 1);
    m_data.insert(row + 1, m_data[row]);
    m_time.insert(row + 1, m_time[row]);
    m_status.insert(row + 1, m_status[row]);
    endInsertRows();
}

void ExamTableModel::removeRow(int row)
{
    if (row < 0 || row >= m_data.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    m_time.removeAt(row);
    m_status.removeAt(row);
    endRemoveRows();
}

QJsonObject ExamTableModel::getExamData(int row)
{
    if (row < 0 || row >= m_data.size())
    {
        qDebug() << "getExamData: wrong row: " << row;
        return QJsonObject();
    }

    return m_data[row].toObject();
}

void ExamTableModel::setExamParams(int row, QJsonObject parameters)
{
    if (row < 0 || row >= m_data.size())
    {
        qDebug() << "getExamData: wrong row: " << row;
        return;
    }

    if(m_status[row]!=0) // 0 is Ready
    {
        qDebug() << "unexpected action: changed scanned sequence" << row;
    }

    QJsonObject exam = m_data[row].toObject();
    exam["parameters"] = parameters;
    m_data.replace(row, exam);
}
