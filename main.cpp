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

    // 初始化日志系统
    Logger::setLogToFile(true);
    Logger::setMinLogLevel(LogLevel::Debug);
    LOG_INFO("应用程序启动");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "mrscan2_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            LOG_INFO(QString("加载语言: %1").arg(locale));
            break;
        }
    }

    // 确保配置目录存在
    QDir configDir("./configs");
    if (!configDir.exists()) {
        configDir.mkpath(".");
        LOG_INFO("创建配置目录");
    }

    CustomPreferences::setupApp();
    LOG_INFO("自定义配置设置完成");

    // 可以在这里创建自定义的扫描仪适配器或者使用默认实现
    // IScannerAdapter* customAdapter = new CustomScannerAdapter();
    // MainWindow w(nullptr, customAdapter);
    
    MainWindow w;
    w.show();
    LOG_INFO("主窗口显示");

    int result = a.exec();
    LOG_INFO(QString("应用程序退出，返回码：%1").arg(result));
    return result;
}
