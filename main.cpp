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

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "mrscan2_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            LOG_INFO(QString("Loaded language: %1").arg(locale));
            break;
        }
    }

    config::Appearance::setupApp();

    MainWindow w;
    w.show();
    LOG_INFO("Main window displayed");

    int result = a.exec();
    LOG_INFO(QString("Application exited with code: %1").arg(result));
    return result;
}
