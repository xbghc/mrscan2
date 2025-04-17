#include "mainwindow.h"
#include "custompreferences.h"
#include "utils.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize log system
    Logger::setLogToFile(true);
    Logger::setMinLogLevel(LogLevel::Debug);
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

    // Ensure config directory exists
    QDir configDir("./configs");
    if (!configDir.exists()) {
        configDir.mkpath(".");
        LOG_INFO("Created config directory");
    }

    CustomPreferences::setupApp();
    LOG_INFO("Custom preferences setup completed");

    // You can create a custom scanner adapter here or use the default implementation
    // IScannerAdapter* customAdapter = new CustomScannerAdapter();
    // MainWindow w(nullptr, customAdapter);
    
    MainWindow w;
    w.show();
    LOG_INFO("Main window displayed");

    int result = a.exec();
    LOG_INFO(QString("Application exited with code: %1").arg(result));
    return result;
}
