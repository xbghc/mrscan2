#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <fftw3.h>
#include <memory>


// Log levels
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Macros to simplify logging calls
#define LOG_DEBUG(msg) Logger::log(LogLevel::Debug, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::log(LogLevel::Info, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::log(LogLevel::Warning, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::log(LogLevel::Error, msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::log(LogLevel::Critical, msg, __FILE__, __LINE__)

// Global logger class
class Logger {
public:
    static void log(LogLevel level, const QString& message, const char* file = nullptr, int line = 0);
    static void setLogToFile(bool logToFile, const QString& filePath = "logs/mrscan.log");
    static void setMinLogLevel(LogLevel level);

private:
    static bool s_logToFile;
    static QString s_logFilePath;
    static LogLevel s_minLogLevel;
};

// Global error handling class
class ErrorHandler {
public:
    static void handleError(const QString& message, const QString& details = QString());
    static bool showErrorDialog(const QString& message, const QString& details = QString());
};



/**
 * @brief 对fftw库的一些封装，也包括相关结构，如数组等的封装
 * @todo 封装确保fftw_complex的内存自动释放
 * @detail 这里的vector使用标准库，没有使用QVector因为方便以后在非Qt的C++项目中使用
 */
namespace fftw_utils{
    // 定义一个自定义的deleter，用于unique_ptr
    struct FFTWDeleter {
        void operator()(fftw_complex* ptr) const {
            if (ptr) {
                fftw_free(ptr);
            }
        }
    };

    // 使用 unique_ptr 和自定义 deleter 来管理 fftw_complex 内存
    using fftw_complex_ptr = std::unique_ptr<fftw_complex[], FFTWDeleter>;

    fftw_complex_ptr createArray(size_t size);

    std::vector<double> abs(fftw_complex* array, size_t len);

    fftw_complex_ptr exec_fft_3d(fftw_complex* in, std::vector<int> n);

    /**
     * @brief 用于逻辑上的多维数组，但是使用一维数组表示，根据数组形状和每个维度的索引给出数组索引
     * @param shape 数组的形状
     * @param indices 各个维度的下标
     */
    int getIndex(std::vector<int> shape, std::vector<int> indices);

    void fftshift3d(fftw_complex* data, std::vector<int> shape);
} // namespace FFTW


namespace file_utils{

QByteArray read(const QString& fpath);

void save(const QString& fpath, QByteArray content);

}

namespace json_utils{
    QString get(const QJsonObject& obj, const QString key, QString d);
    int get(const QJsonObject& obj, const QString key, int d);
    QJsonDocument readFromFile(const QString& fpath);
    void saveToFile(const QString& fpath, QJsonObject obj);
    void saveToFile(const QString& fpath, QJsonArray array);
}

namespace utils{
    QString secondsToString(int seconds);
}

#endif // UTILS_H
