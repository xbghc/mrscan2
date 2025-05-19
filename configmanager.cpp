#include "configmanager.h"
#include "utils.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

const QString ConfigManager::kConfigDir = "./configs";

namespace {
QJsonArray defaultExamList(){
    QJsonDocument doc = QJsonDocument::fromJson(R"([{
      "name" : "scout",
        "sequence" : "t2",
        "parameters" : {
                       "observeFrequency" : 0,
            "noSamples" : 128,
            "noViews" : 128,
            "viewsPerSegment" : 1,
            "noAverages" : 4,
            "sliceThickness" : 10,
            "fov" : 256,
            "noSlices" : 9,
            "slices" : [
                {
                 "xOffset" : 0,
                    "yOffset" : 0,
                    "zOffset" : -90,
                    "xAngle" : 0,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : 0,
                    "zOffset" : 0,
                    "xAngle" : 0,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : 0,
                    "zOffset" : 90,
                    "xAngle" : 0,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : -90,
                    "zOffset" : 0,
                    "xAngle" : 90,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : 0,
                    "zOffset" : 0,
                    "xAngle" : 90,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : 90,
                    "zOffset" : 0,
                    "xAngle" : 90,
                    "yAngle" : 0,
                    "zAngle" : 0
                },
                {
                 "xOffset" : -90,
                    "yOffset" : 0,
                    "zOffset" : 0,
                    "xAngle" : 0,
                    "yAngle" : 90,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 0,
                    "yOffset" : 0,
                    "zOffset" : 0,
                    "xAngle" : 0,
                    "yAngle" : 90,
                    "zAngle" : 0
                },
                {
                 "xOffset" : 90,
                    "yOffset" : 0,
                    "zOffset" : 0,
                    "xAngle" : 0,
                    "yAngle" : 90,
                    "zAngle" : 0
                }
            ]
        }
    },
     {
      "name" : "T1",
         "sequence" : "t1",
         "parameters" : {
                        "observeFrequency" : 0,
             "noSamples" : 128,
             "noViews" : 128,
             "noViews2" : 18,
             "noAverages" : 4,
             "sliceThickness" : 10,
             "sliceSeparation" : 12,
             "fov" : 256,
             "noSlices" : 1,
             "xAngle" : 0,
             "yAngle" : 0,
             "zAngle" : 0,
             "xOffset" : 0,
             "yOffset" : 0,
             "zOffset" : 0
         }
     },
     {
      "name" : "T2",
         "sequence" : "t2",
         "parameters" : {
                        "observeFrequency" : 0,
             "noSamples" : 128,
             "noViews" : 128,
             "viewsPerSegment" : 1,
             "noAverages" : 4,
             "sliceThickness" : 10,
             "fov" : 256,
             "noSlices" : 9,
             "slices" : [
                 {
                  "xOffset" : 0,
                     "yOffset" : 0,
                     "zOffset" : -90,
                     "xAngle" : 0,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : 0,
                     "zOffset" : 0,
                     "xAngle" : 0,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : 0,
                     "zOffset" : 90,
                     "xAngle" : 0,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : -90,
                     "zOffset" : 0,
                     "xAngle" : 90,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : 0,
                     "zOffset" : 0,
                     "xAngle" : 90,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : 90,
                     "zOffset" : 0,
                     "xAngle" : 90,
                     "yAngle" : 0,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : -90,
                     "yOffset" : 0,
                     "zOffset" : 0,
                     "xAngle" : 0,
                     "yAngle" : 90,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 0,
                     "yOffset" : 0,
                     "zOffset" : 0,
                     "xAngle" : 0,
                     "yAngle" : 90,
                     "zAngle" : 0
                 },
                 {
                  "xOffset" : 90,
                     "yOffset" : 0,
                     "zOffset" : 0,
                     "xAngle" : 0,
                     "yAngle" : 90,
                     "zAngle" : 0
                 }
             ]
         }
     }])");

    return doc.array();
}
}

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
    json_utils::saveToFile(fpath(cname), m_configs[cname].toObject());
}

QJsonValue ConfigManager::get(const QString &cname, const QString &key) {
    if (!m_configs.contains(cname)) {
        load(cname);
    }

    return m_configs[cname].toObject().value(key);
}

void ConfigManager::set(const QString &cname, const QString &key,
                        QJsonValue value) {
    m_configs[cname].toObject()[key] = value;
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
