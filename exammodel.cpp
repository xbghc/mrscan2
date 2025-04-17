#include "exammodel.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QMutexLocker>

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
    : QAbstractTableModel{parent}
    , m_scanningRow(-1)
    , m_timerThread(nullptr)
    , m_threadShouldExit(false)
{
    m_headers << "Sequence" << "Time" << "Status";
    loadExams();
}

ExamModel::~ExamModel() {
    // Ensure thread is stopped and data is saved
    if (m_timerThread != nullptr) {
        stopTimerThread();
    }
    resetScanningState();
    saveExams();
}

// New method to reset scanning state
void ExamModel::resetScanningState() {
    QMutexLocker locker(&m_mutex);
    m_scanningRow = -1;
    LOG_DEBUG("Scanning state has been reset");
}

int ExamModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_exams.size();
}

int ExamModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return ColumnCount;
}

QVariant ExamModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int row = index.row();
    if (!isValidRow(row))
        return QVariant();

    int col = index.column();
    const ExamItem& exam = m_exams[row];

    switch (col) {
    case NameColumn:
        return exam.name();
    case TimeColumn:
        return exam.time();
    case StatusColumn:
        return ExamItem::statusText(exam.status());
    default:
        return QVariant();
    }
}

QVariant ExamModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section < m_headers.size()) {
        return m_headers[section];
    }
    return QVariant();
}

void ExamModel::swapRows(int row1, int row2) {
    if (!isValidRow(row1) || !isValidRow(row2) || row1 == row2)
        return;

    beginMoveRows(QModelIndex(), row1, row1, QModelIndex(), row2 > row1 ? row2 + 1 : row2);
    m_exams.swapItemsAt(row1, row2);
    endMoveRows();
}

void ExamModel::copyRow(int row) {
    if (!isValidRow(row))
        return;

    beginInsertRows(QModelIndex(), row + 1, row + 1);
    m_exams.insert(row + 1, m_exams[row]);
    m_exams[row + 1].setStatus(ExamItem::Status::Ready);
    endInsertRows();
}

bool ExamModel::removeRow(int row, const QModelIndex &parent) {
    Q_UNUSED(parent);
    
    if (!isValidRow(row))
        return false;

    // Handle scanning state
    bool needStopTimer = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_scanningRow == row) {
            needStopTimer = true;
        }
    }
    
    // If needed, stop the timer thread outside the lock
    if (needStopTimer) {
        stopTimerThread();
        resetScanningState();
    }

    // Update row index, this is a UI model operation, no lock needed
    beginRemoveRows(QModelIndex(), row, row);
    m_exams.removeAt(row);
    endRemoveRows();
    
    // Update scanning row index
    {
        QMutexLocker locker(&m_mutex);
        if (m_scanningRow > row) {
            m_scanningRow--;
        }
    }
    
    return true;
}

QJsonObject ExamModel::getExamData(int row) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Invalid row index when getting exam data: %1").arg(row));
        return QJsonObject();
    }

    return m_exams[row].data();
}

void ExamModel::setExamParams(int row, const QJsonObject& parameters) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Invalid row index when setting exam parameters: %1").arg(row));
        return;
    }

    if (m_exams[row].status() != ExamItem::Status::Ready) {
        LOG_WARNING(QString("Attempting to modify parameters of non-ready exam, row: %1").arg(row));
    }

    m_exams[row].setParameters(parameters);
    QModelIndex index = createIndex(row, NameColumn);
    emit dataChanged(index, index);
}

void ExamModel::setExamResponse(int row, const QJsonObject& response) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Invalid row index when setting exam response: %1").arg(row));
        return;
    }

    m_exams[row].setResponse(response);
}

void ExamModel::examStarted(int row, int id) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Invalid row index when exam starts: %1").arg(row));
        return;
    }

    if (id < 0) {
        LOG_ERROR(QString("Exam started but ID is invalid: %1").arg(id));
        return;
    }

    // Check timer thread status and set scanning state
    bool canStartTimer = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_timerThread != nullptr) {
            LOG_ERROR("Timer thread already exists, cannot start");
            return;
        }
        
        // Set ID and state
        m_exams[row].setId(id);
        m_scanningRow = row;
        canStartTimer = true;
    }
    
    // Update UI model, no lock needed
    m_exams[row].setStatus(ExamItem::Status::Processing);
    QModelIndex index = createIndex(row, StatusColumn);
    emit dataChanged(index, index);
    emit examStatusChanged(row, ExamItem::Status::Processing);
    
    // If possible, start timer outside lock
    if (canStartTimer) {
        startTimerThread();
    }
}

int ExamModel::getScanningRow() {
    QMutexLocker locker(&m_mutex);
    return m_scanningRow;
}

int ExamModel::getScanningId() {
    QMutexLocker locker(&m_mutex);
    if (m_scanningRow >= 0 && m_scanningRow < m_exams.size()) {
        return m_exams[m_scanningRow].id();
    }
    return -1;
}

int ExamModel::examStoped() {
    QMutexLocker locker(&m_mutex);
    int row = m_scanningRow;
    locker.unlock();
    
    if (row < 0 || row >= m_exams.size()) {
        LOG_WARNING("Cannot stop exam: no exam in progress");
        return -1;
    }
    
    return finishExam(row, ExamItem::Status::Ready, false);
}

int ExamModel::examDone() {
    QMutexLocker locker(&m_mutex);
    int row = m_scanningRow;
    locker.unlock();
    
    if (row < 0 || row >= m_exams.size()) {
        LOG_WARNING("Cannot complete exam: no exam in progress");
        return -1;
    }
    
    return finishExam(row, ExamItem::Status::Done, true);
}

bool ExamModel::loadExams(const QString& filePath) {
    m_exams = loadExamsFromFile(filePath);
    beginResetModel();
    endResetModel();
    return !m_exams.isEmpty();
}

bool ExamModel::saveExams(const QString& filePath) const {
    QDir dir("./configs");
    if (!dir.exists() && !dir.mkpath(".")) {
        LOG_ERROR(QString("Cannot create config directory: %1").arg(dir.path()));
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Cannot write config file: %1").arg(filePath));
        return false;
    }

    QJsonArray examsArray;
    for (const ExamItem& exam : m_exams) {
        examsArray.append(exam.data());
    }

    QJsonDocument doc(examsArray);
    file.write(doc.toJson());
    file.close();
    
    LOG_INFO(QString("Exam config saved: %1").arg(filePath));
    return true;
}

void ExamModel::updateTimers() {
    int currentRow;
    
    {
        QMutexLocker locker(&m_mutex);
        currentRow = m_scanningRow;
        
        if (currentRow < 0 || currentRow >= m_exams.size())
            return;
    }
    
    // Calculate time
    int seconds = m_startTime.secsTo(QTime::currentTime());
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    QString timeStr = QString("%1:%2").arg(m, 1, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    
    // Update UI model
    m_exams[currentRow].setTime(timeStr);
    QModelIndex index = createIndex(currentRow, TimeColumn);
    emit dataChanged(index, index);
    emit timerUpdated(currentRow, timeStr);
}

bool ExamModel::isValidRow(int row) const {
    return row >= 0 && row < m_exams.size();
}

void ExamModel::startTimerThread() {
    QThread* newThread = nullptr;
    int scanningRow;
    
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_timerThread != nullptr) {
            LOG_WARNING("Timer thread already exists, cannot start again");
            return;
        }
        
        // Set start time
        m_startTime = QTime::currentTime();
        m_threadShouldExit = false;
        scanningRow = m_scanningRow;
        
        // Create new thread
        newThread = QThread::create([this, scanningRow]() {
            while (true) {
                bool shouldExit;
                int currentRow;
                
                {
                    QMutexLocker threadLocker(&m_mutex);
                    shouldExit = m_threadShouldExit;
                    currentRow = m_scanningRow;
                }
                
                // Check if should exit
                if (shouldExit || currentRow == -1 || currentRow != scanningRow) {
                    break;
                }
                
                // Update timer outside lock
                QMetaObject::invokeMethod(this, "updateTimers", Qt::QueuedConnection);
                QThread::msleep(1000); // Update every second
            }
            
            LOG_DEBUG("Timer thread exited normally");
        });
        
        m_timerThread = newThread;
    }
    
    // Start thread outside lock
    if (newThread) {
        newThread->start();
        LOG_DEBUG("Timer thread started");
    }
}

void ExamModel::stopTimerThread() {
    QThread* threadToStop = nullptr;
    
    {
        QMutexLocker locker(&m_mutex);
        if (m_timerThread == nullptr) {
            return;
        }
        
        // Set exit flag
        m_threadShouldExit = true;
        threadToStop = m_timerThread;
        m_timerThread = nullptr;
    }
    
    // Wait for thread to complete outside lock
    if (threadToStop) {
        threadToStop->wait();
        delete threadToStop;
        LOG_DEBUG("Timer thread stopped");
    }
}

QList<ExamItem> ExamModel::loadExamsFromFile(const QString& filePath) const {
    QList<ExamItem> exams;
    
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        LOG_WARNING(QString("No exam config file: %1").arg(filePath));
        return exams;
    }

    QJsonArray examsArray = QJsonDocument::fromJson(file.readAll()).array();
    
    for (const QJsonValue& value : examsArray) {
        if (value.isObject()) {
            exams.append(ExamItem(value.toObject()));
        }
    }
    
    LOG_INFO(QString("Loaded %1 exams").arg(exams.size()));
    return exams;
}

int ExamModel::finishExam(int row, ExamItem::Status newStatus, bool shouldSave) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Failed to finish/stop exam: invalid row index %1").arg(row));
        return -1;
    }
    
    // Check exam status outside lock to reduce lock time
    if (m_exams[row].status() != ExamItem::Status::Processing) {
        QString statusText = ExamItem::statusText(m_exams[row].status());
        LOG_WARNING(QString("Exam %1 status error (current status: %2), cannot change status")
                  .arg(m_exams[row].name())
                  .arg(statusText));
        return -1;
    }
    
    // Get ID, no lock needed
    int id = m_exams[row].id();
    
    // Stop timer thread
    stopTimerThread();
    
    // Update status outside lock, as this is a UI model update
    m_exams[row].setStatus(newStatus);
    QModelIndex index = createIndex(row, StatusColumn);
    emit dataChanged(index, index);
    emit examStatusChanged(row, newStatus);
    
    // Reset scanning state needs lock
    resetScanningState();
    
    // Save config if needed
    if (shouldSave) {
        saveExams();
    }
    
    LOG_INFO(QString("Exam %1: ID=%2, Name=\"%3\"")
            .arg(newStatus == ExamItem::Status::Done ? "completed" : "stopped")
            .arg(id)
            .arg(m_exams[row].name()));
    
    return id;
}
