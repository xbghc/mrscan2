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
    m_headers << "序列" << "时间" << "状态";
    loadExams();
}

ExamModel::~ExamModel() {
    // 确保停止线程并保存数据
    if (m_timerThread != nullptr) {
        stopTimerThread();
    }
    resetScanningState();
    saveExams();
}

// 重置扫描状态的新方法
void ExamModel::resetScanningState() {
    QMutexLocker locker(&m_mutex);
    m_scanningRow = -1;
    LOG_DEBUG("扫描状态已重置");
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

    // 处理扫描状态
    bool needStopTimer = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_scanningRow == row) {
            needStopTimer = true;
        }
    }
    
    // 如果需要，在锁外停止计时线程
    if (needStopTimer) {
        stopTimerThread();
        resetScanningState();
    }

    // 更新行索引，这是UI模型操作，不需要锁
    beginRemoveRows(QModelIndex(), row, row);
    m_exams.removeAt(row);
    endRemoveRows();
    
    // 更新扫描行索引
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
        LOG_ERROR(QString("获取检查数据时行索引无效: %1").arg(row));
        return QJsonObject();
    }

    return m_exams[row].data();
}

void ExamModel::setExamParams(int row, const QJsonObject& parameters) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("设置检查参数时行索引无效: %1").arg(row));
        return;
    }

    if (m_exams[row].status() != ExamItem::Status::Ready) {
        LOG_WARNING(QString("尝试修改非就绪状态的检查参数，行: %1").arg(row));
    }

    m_exams[row].setParameters(parameters);
    QModelIndex index = createIndex(row, NameColumn);
    emit dataChanged(index, index);
}

void ExamModel::setExamResponse(int row, const QJsonObject& response) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("设置检查响应时行索引无效: %1").arg(row));
        return;
    }

    m_exams[row].setResponse(response);
}

void ExamModel::examStarted(int row, int id) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("检查开始时行索引无效: %1").arg(row));
        return;
    }

    if (id < 0) {
        LOG_ERROR(QString("检查开始但ID无效: %1").arg(id));
        return;
    }

    // 检查计时线程状态并设置扫描状态
    bool canStartTimer = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_timerThread != nullptr) {
            LOG_ERROR("计时线程已存在，无法启动");
            return;
        }
        
        // 设置ID和状态
        m_exams[row].setId(id);
        m_scanningRow = row;
        canStartTimer = true;
    }
    
    // 更新UI模型，不需要锁
    m_exams[row].setStatus(ExamItem::Status::Processing);
    QModelIndex index = createIndex(row, StatusColumn);
    emit dataChanged(index, index);
    emit examStatusChanged(row, ExamItem::Status::Processing);
    
    // 如果可以，在锁外启动计时器
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
        LOG_WARNING("无法停止检查：没有正在进行的检查");
        return -1;
    }
    
    return finishExam(row, ExamItem::Status::Ready, false);
}

int ExamModel::examDone() {
    QMutexLocker locker(&m_mutex);
    int row = m_scanningRow;
    locker.unlock();
    
    if (row < 0 || row >= m_exams.size()) {
        LOG_WARNING("无法完成检查：没有正在进行的检查");
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
        LOG_ERROR(QString("无法创建配置目录: %1").arg(dir.path()));
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("无法写入配置文件: %1").arg(filePath));
        return false;
    }

    QJsonArray examsArray;
    for (const ExamItem& exam : m_exams) {
        examsArray.append(exam.data());
    }

    QJsonDocument doc(examsArray);
    file.write(doc.toJson());
    file.close();
    
    LOG_INFO(QString("检查配置已保存: %1").arg(filePath));
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
    
    // 计算时间
    int seconds = m_startTime.secsTo(QTime::currentTime());
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    QString timeStr = QString("%1:%2").arg(m, 1, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    
    // 更新UI模型
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
            LOG_WARNING("计时线程已存在，无法再次启动");
            return;
        }
        
        // 设置开始时间
        m_startTime = QTime::currentTime();
        m_threadShouldExit = false;
        scanningRow = m_scanningRow;
        
        // 创建新线程
        newThread = QThread::create([this, scanningRow]() {
            while (true) {
                bool shouldExit;
                int currentRow;
                
                {
                    QMutexLocker threadLocker(&m_mutex);
                    shouldExit = m_threadShouldExit;
                    currentRow = m_scanningRow;
                }
                
                // 判断是否应该退出
                if (shouldExit || currentRow == -1 || currentRow != scanningRow) {
                    break;
                }
                
                // 在锁外更新计时
                QMetaObject::invokeMethod(this, "updateTimers", Qt::QueuedConnection);
                QThread::msleep(1000); // 每秒更新一次
            }
            
            LOG_DEBUG("计时线程正常退出");
        });
        
        m_timerThread = newThread;
    }
    
    // 在锁外启动线程
    if (newThread) {
        newThread->start();
        LOG_DEBUG("计时线程已启动");
    }
}

void ExamModel::stopTimerThread() {
    QThread* threadToStop = nullptr;
    
    {
        QMutexLocker locker(&m_mutex);
        if (m_timerThread == nullptr) {
            return;
        }
        
        // 设置退出标志
        m_threadShouldExit = true;
        threadToStop = m_timerThread;
        m_timerThread = nullptr;
    }
    
    // 在锁外等待线程完成
    if (threadToStop) {
        threadToStop->wait();
        delete threadToStop;
        LOG_DEBUG("计时线程已停止");
    }
}

QList<ExamItem> ExamModel::loadExamsFromFile(const QString& filePath) const {
    QList<ExamItem> exams;
    
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        LOG_WARNING(QString("无检查配置文件: %1").arg(filePath));
        return exams;
    }

    QJsonArray examsArray = QJsonDocument::fromJson(file.readAll()).array();
    
    for (const QJsonValue& value : examsArray) {
        if (value.isObject()) {
            exams.append(ExamItem(value.toObject()));
        }
    }
    
    LOG_INFO(QString("已加载%1个检查").arg(exams.size()));
    return exams;
}

int ExamModel::finishExam(int row, ExamItem::Status newStatus, bool shouldSave) {
    if (!isValidRow(row)) {
        LOG_ERROR(QString("完成/停止检查失败：行索引 %1 无效").arg(row));
        return -1;
    }
    
    // 锁外进行状态检查，减少持锁时间
    if (m_exams[row].status() != ExamItem::Status::Processing) {
        QString statusText = ExamItem::statusText(m_exams[row].status());
        LOG_WARNING(QString("检查 %1 状态错误（当前状态: %2），无法更改状态")
                  .arg(m_exams[row].name())
                  .arg(statusText));
        return -1;
    }
    
    // 获取ID，不需要锁
    int id = m_exams[row].id();
    
    // 停止计时线程
    stopTimerThread();
    
    // 更新状态不需要锁，因为这是UI模型更新
    m_exams[row].setStatus(newStatus);
    QModelIndex index = createIndex(row, StatusColumn);
    emit dataChanged(index, index);
    emit examStatusChanged(row, newStatus);
    
    // 重置扫描状态需要锁
    resetScanningState();
    
    // 保存配置，如果需要
    if (shouldSave) {
        saveExams();
    }
    
    LOG_INFO(QString("检查%1：ID=%2, 名称=\"%3\"")
            .arg(newStatus == ExamItem::Status::Done ? "完成" : "停止")
            .arg(id)
            .arg(m_exams[row].name()));
    
    return id;
}
