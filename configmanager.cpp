#include "configmanager.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

const QString ConfigManager::kConfigDir = "./configs";

namespace {
QJsonArray defaultExamList(){
    const QString kDefaultExamsPath = ":/configs/default_exams.json";

    auto doc = json_utils::readFromFile(kDefaultExamsPath);
    if (doc.isNull() || !doc.isArray()) {
        LOG_ERROR("Failed to load or parse default_exams.json. Returning empty array.");
        return QJsonArray();
    }

    return doc.array();
}
} // namespace


ConfigManager *ConfigManager::instance() {
    static ConfigManager s_instance;
    return &s_instance;
}

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {
    // Ensure config directory exists
    QDir configDir(kConfigDir);
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            LOG_ERROR("Failed to create config directory");
        }
    }
}

ConfigManager::~ConfigManager() {}

void ConfigManager::load(const QString &cname) {
    m_configs[cname] = json_utils::readFromFile(fpath(cname)).object();
}

void ConfigManager::save(const QString &cname) {
    json_utils::saveToFile(fpath(cname), m_configs[cname]);
}

QJsonValue ConfigManager::get(const QString &cname, const QString &key) {
    if (!m_configs.contains(cname)) {
        load(cname);
    }

    return m_configs[cname].value(key);
}

void ConfigManager::set(const QString &cname, const QString &key,
                        QJsonValue value) {
    m_configs[cname][key] = value;
    save(cname);
}

QString ConfigManager::fpath(const QString &fname) {
    return QString("%1/%2.json").arg(kConfigDir, fname);
}

const QString ExamConfig::kName = "exam";
const QString ExamConfig::Keys::InitExams = "initExams";

QList<Exam> ExamConfig::initialExams() {
    auto cm = ConfigManager::instance();
    auto array = cm->get(kName, Keys::InitExams).toArray();
     if (array.empty()) {
        array = defaultExamList();
        cm->set(kName, Keys::InitExams, array);
    }

    QList<Exam> exams;
    for (int i = 0; i < array.size(); i++) {
        auto obj = array[i].toObject();
        Exam exam;
        ExamRequest request(obj);
        exam.setRequest(request);
        exams.push_back(exam);
    }

    return exams;
}
