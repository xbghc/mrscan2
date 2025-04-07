#include "exammodel.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>

namespace {
QJsonArray loadExams() {
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
} // namespace

ExamModel::ExamModel(QObject *parent)
    : QAbstractTableModel{parent}, timerThread(nullptr) {
    m_data = loadExams();
    for (int i = 0; i < m_data.size(); i++) {
        m_time.append(QString("0:00"));
        m_status.append(ExamStatus::Ready);
    }

    m_headers << "Sequence" << "Time" << "Status";
}

ExamModel::~ExamModel() {
    if (timerThread == nullptr) {
        return;
    }

    for(auto& s:m_status){
        s = ExamStatus::Ready;
    }
    timerThread->wait();
    delete timerThread;
}

int ExamModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_data.size();
}

int ExamModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_headers.size();
}

QVariant ExamModel::data(const QModelIndex &index, int role) const {
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
        return kStatus[static_cast<int>(m_status[row])];
    default:
        return QVariant();
    }
}

QVariant ExamModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_headers[section];
    }
    return QVariant();
}

void ExamModel::swapRows(int row1, int row2) {
    if (row1 < 0 || row1 >= m_data.size() || row2 < 0 || row2 >= m_data.size() ||
        row1 == row2)
        return;

    beginMoveRows(QModelIndex(), row1, row1, QModelIndex(),
                  row2 > row1 ? row2 + 1 : row2);

    QJsonValue temp1 = m_data[row1];
    QJsonValue temp2 = m_data[row2];
    m_data.replace(row1, temp2);
    m_data.replace(row2, temp1);

    m_time.swapItemsAt(row1, row2);
    m_status.swapItemsAt(row1, row2);
    endMoveRows();
}

void ExamModel::copyRow(int row) {
    if (row < 0 || row >= m_data.size())
        return;

    beginInsertRows(QModelIndex(), row + 1, row + 1);
    m_data.insert(row + 1, m_data[row]);
    m_time.insert(row + 1, m_time[row]);
    m_status.insert(row + 1, ExamStatus::Ready);
    endInsertRows();
}

void ExamModel::removeRow(int row) {
    if (row < 0 || row >= m_data.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    m_time.removeAt(row);
    m_status.removeAt(row);
    endRemoveRows();
}

QJsonObject ExamModel::getExamData(int row) {
    if (row < 0 || row >= m_data.size()) {
        qDebug() << "getExamData: wrong row: " << row;
        return QJsonObject();
    }

    return m_data[row].toObject();
}

void ExamModel::setExamParams(int row, QJsonObject parameters) {
    if (row < 0 || row >= m_data.size()) {
        qDebug() << "getExamData: wrong row: " << row;
        return;
    }

    if (m_status[row] != ExamStatus::Ready)
    {
        qDebug() << "unexpected action: changed scanned sequence" << row;
    }

    QJsonObject exam = m_data[row].toObject();
    exam["parameters"] = parameters;
    m_data.replace(row, exam);
}

void ExamModel::setExamResponse(int row, QJsonObject response)
{
    if (row < 0 || row >= m_data.size()) {
        qDebug() << "getExamData: wrong row: " << row;
        return;
    }

    QJsonObject exam = m_data[row].toObject();
    exam["response"] = response;
    m_data.replace(row, exam);
}

void ExamModel::examStarted(int row, int id) {
    if (row < 0 || row >= m_data.size()) {
        qDebug() << "getExamData: wrong row: " << row;
        return;
    }

    if (id < 0) {
        qDebug() << "exam started but id < 0";
    }

    if (timerThread != nullptr) {
        qDebug() << "timers already exist, can't start";
        return;
    }

    QJsonObject exam = m_data[row].toObject();
    exam["id"] = id;
    m_data.replace(row, exam);
    beginResetModel();
    m_status[row] = ExamStatus::Processing;
    endResetModel();

    timerThread = QThread::create([this]() {
        QTime startTime = QTime::currentTime();

        for(int scanningRow = this->getScanningRow();scanningRow != -1;scanningRow = this->getScanningRow()) {
            int seconds = startTime.secsTo(QTime::currentTime());
            int m = (seconds % 3600) / 60;
            int s = seconds % 60;
            this->m_time[scanningRow] =
                QString("%1:%2").arg(m, 1, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
            QModelIndex index = this->createIndex(scanningRow, 1);
            emit this->dataChanged(index, index);
            QThread::msleep(200);
        }
    });
    connect(timerThread, &QThread::finished, timerThread, &QThread::deleteLater);
    connect(timerThread, &QThread::destroyed, this, [this]() {
        timerThread = nullptr;
    });
    timerThread->start();
}

int ExamModel::getScanningRow()
{
    for(int row=0;row<rowCount();row++){
        if(m_status[row] == ExamStatus::Processing){
            return row;
        }
    }
    return -1;
}

int ExamModel::getScanningId() {
    int row = getScanningRow();
    if (row < 0) {
        return -1;
    }

    QJsonObject exam = m_data[row].toObject();
    if (!exam.contains("id")) {
        qDebug() << "scanning item doesn't have id";
        return -1;
    }

    return exam["id"].toInt();
}

int ExamModel::examStoped()
{
    int row = getScanningRow();
    if(row < 0 || m_status[row] != ExamStatus::Processing){
        qDebug() << "noting to stop";
        return -1;
    }

    m_status[row] = ExamStatus::Ready;
    QJsonObject exam = m_data[row].toObject();
    return exam["id"].toInt();
}

int ExamModel::examDone()
{
    int row = getScanningRow();
    if(row < 0 || m_status[row] != ExamStatus::Processing){
        qDebug() << "noting to stop";
        return -1;
    }

    m_status[row] = ExamStatus::Done;
    QJsonObject exam = m_data[row].toObject();
    return exam["id"].toInt();
}
