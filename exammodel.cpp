#include "exammodel.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutexLocker>

namespace {
QJsonArray loadExams() {
    const static QString kPath = "./configs/exams.json";

    QDir dir("./configs");
    if (!dir.exists() && dir.mkpath(".")) {
        LOG_WARNING("Failed to mkdir: " + dir.path());
        return QJsonArray();
    }

    QFile file(kPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        LOG_WARNING("No Exam Configuration!");
        return QJsonArray();
    }

    QJsonArray exams = QJsonDocument::fromJson(file.readAll()).array();
    return exams;
}
} // namespace

ExamModel::ExamModel(QObject *parent)
    : QAbstractTableModel{parent}
    , m_scanningRow(-1)
    , m_threadShouldExit(false)
{
    m_headers << "Sequence" << "Time" << "Status";
    loadExams();
}

ExamModel::~ExamModel() {
    // Ensure thread is stopped and data is saved
    stopTimerThread();
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
        if (m_timerThread) {
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
    QList<ExamItem> loadedExams = loadExamsFromFile(filePath);
    
    if (loadedExams.isEmpty()) {
        LOG_ERROR(QString("Failed to load exams from file: %1").arg(filePath));
        return false;
    }
    
    beginResetModel();
    m_exams = loadedExams;
    endResetModel();
    
    LOG_INFO(QString("Loaded %1 exams from %2").arg(m_exams.size()).arg(filePath));
    return true;
}

bool ExamModel::saveExams(const QString& filePath) const {
    if (m_exams.isEmpty()) {
        LOG_WARNING("No exams to save");
        return false;
    }
    
    // Ensure directory exists
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        LOG_ERROR(QString("Failed to create directory for exams file: %1").arg(dir.path()));
        return false;
    }
    
    // Convert exams to JSON array
    QJsonArray examsArray;
    for (const ExamItem& exam : m_exams) {
        examsArray.append(exam.toJsonObject());
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("Cannot open exams file for writing: %1").arg(filePath));
        return false;
    }
    
    QJsonDocument doc(examsArray);
    file.write(doc.toJson());
    file.close();
    
    LOG_INFO(QString("Saved %1 exams to %2").arg(m_exams.size()).arg(filePath));
    return true;
}

void ExamModel::updateTimers() {
    QMutexLocker locker(&m_mutex);
    
    if (m_threadShouldExit) {
        return;
    }
    
    int currentRow = m_scanningRow;
    int elapsedMs = m_startTime.elapsed();
    
    // Release the lock before emitting signals
    locker.unlock();
    
    if (currentRow >= 0 && currentRow < m_exams.size()) {
        int seconds = elapsedMs / 1000;
        int minutes = seconds / 60;
        seconds = seconds % 60;
        
        QString timeStr = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        m_exams[currentRow].setTime(timeStr);
        
        QModelIndex index = createIndex(currentRow, TimeColumn);
        emit dataChanged(index, index);
        emit timerUpdated(currentRow, timeStr);
    }
}

bool ExamModel::isValidRow(int row) const {
    return row >= 0 && row < m_exams.size();
}

void ExamModel::startTimerThread() {
    QMutexLocker locker(&m_mutex);
    
    if (m_timerThread) {
        LOG_WARNING("Timer thread already running");
        return;
    }
    
    m_threadShouldExit = false;
    m_startTime.start();
    
    // Create a new thread with lambda
    m_timerThread = std::make_unique<QThread>();
    
    // Create timer and move to thread
    QTimer* timer = new QTimer();
    timer->setInterval(1000);  // Update every second
    timer->moveToThread(m_timerThread.get());
    
    // Connect signals/slots
    connect(m_timerThread.get(), &QThread::started, timer, [timer]() {
        timer->start();
    });
    
    connect(m_timerThread.get(), &QThread::finished, timer, &QTimer::deleteLater);
    connect(timer, &QTimer::timeout, this, &ExamModel::updateTimers);
    
    // Start the thread
    m_timerThread->start();
    
    LOG_DEBUG("Timer thread started");
}

void ExamModel::stopTimerThread() {
    // Set the flag to exit and get the thread pointer
    std::unique_ptr<QThread> thread;
    
    {
        QMutexLocker locker(&m_mutex);
        if (!m_timerThread) {
            return;
        }
        m_threadShouldExit = true;
        thread = std::move(m_timerThread);  // Take ownership of the thread
    }
    
    // Stop the thread outside of lock
    if (thread) {
        thread->quit();
        if (!thread->wait(3000)) {  // Wait for 3 seconds
            LOG_WARNING("Timer thread did not quit properly, forcing termination");
            thread->terminate();
            thread->wait();
        }
        
        LOG_DEBUG("Timer thread stopped");
    }
}

QList<ExamItem> ExamModel::loadExamsFromFile(const QString& filePath) const {
    QList<ExamItem> result;
    
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        LOG_WARNING(QString("Exams file does not exist or cannot be opened: %1").arg(filePath));
        return result;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        LOG_ERROR(QString("Exams file is not a valid JSON array: %1").arg(filePath));
        return result;
    }
    
    QJsonArray examsArray = doc.array();
    for (const QJsonValue& value : examsArray) {
        if (!value.isObject()) {
            LOG_WARNING("Skipping invalid exam entry (not an object)");
            continue;
        }
        
        ExamItem item;
        if (item.fromJsonObject(value.toObject())) {
            result.append(item);
        } else {
            LOG_WARNING("Failed to load exam from JSON");
        }
    }
    
    return result;
}

int ExamModel::finishExam(int row, ExamItem::Status newStatus, bool shouldSave) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("Invalid row index when finishing exam: %1").arg(row));
        return -1;
    }
    
    // Stop timer thread
    stopTimerThread();
    
    // Reset scanning state
    resetScanningState();
    
    // Update status
    m_exams[row].setStatus(newStatus);
    
    // Save if needed
    if (shouldSave) {
        saveExams();
    }
    
    // Update UI
    QModelIndex index = createIndex(row, StatusColumn);
    emit dataChanged(index, index);
    emit examStatusChanged(row, newStatus);
    
    return row;
}
