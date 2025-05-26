#include "mainwindow.h"
#include "appearanceconfig.h"
#include "debugconfig.h"
#include "utils.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize log system with debug config
    Logger::setLogToFile(config::Debug::logToFile(), config::Debug::logFilePath());
    Logger::setMinLogLevel(static_cast<LogLevel>(config::Debug::logLevel()));
    LOG_INFO("Application started");

    // Initialize translation system
    QTranslator translator;
    
    // Load translation based on user preference or system language
    QString preferredLanguage = config::Appearance::language();
    bool translationLoaded = false;
    
    if (preferredLanguage == "Chinese") {
        if (translator.load(":/i18n/mrscan2_zh_CN")) {
            a.installTranslator(&translator);
            LOG_INFO("Loaded Chinese translation");
            translationLoaded = true;
        }
    } else {
        // Try system language if preference is not explicitly set
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            if (locale.startsWith("zh")) {
                if (translator.load(":/i18n/mrscan2_zh_CN")) {
                    a.installTranslator(&translator);
                    LOG_INFO(QString("Loaded system language translation: %1").arg(locale));
                    translationLoaded = true;
                    break;
                }
            }
        }
    }
    
    if (!translationLoaded) {
        LOG_INFO("Using default language (English)");
    }

    config::Appearance::setupApp();

    MainWindow w;
    w.show();
    LOG_INFO("Main window displayed");

    int result = a.exec();
    LOG_INFO(QString("Application exited with code: %1").arg(result));
    return result;
}
