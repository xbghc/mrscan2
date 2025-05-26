#include "appearanceconfig.h"
#include "configmanager.h"
#include "utils.h"

#include <QApplication>
#include <QGuiApplication>
#include <QStyle>
#include <QStyleHints>
#include <QStyleFactory>

namespace config{

// 单例模式实现
Appearance* Appearance::instance() {
    static Appearance s_instance;
    return &s_instance;
}

// 构造函数
Appearance::Appearance(QObject *parent) : QObject(parent) {
}

// 静态接口直接实现
int Appearance::fontSize(){
    auto cm = ConfigManager::instance();
    auto fs = cm->get(CONFIG_NAME, KEY_FONT_SIZE);
    if(fs.isNull()){
        cm->set(CONFIG_NAME, KEY_FONT_SIZE, 12);
        return 12;
    }
    return fs.toInt();
}

QString Appearance::fontFamily(){
    auto cm = ConfigManager::instance();
    auto ff = cm->get(CONFIG_NAME, KEY_FONT_FAMILY);
    if(ff.isNull()){
        // 使用系统默认字体，对中文支持更好
        QString defaultFont = QApplication::font().family();
        if (defaultFont.isEmpty()) {
            defaultFont = "Microsoft YaHei"; // Windows 中文环境的默认字体
        }
        cm->set(CONFIG_NAME, KEY_FONT_FAMILY, defaultFont);
        return defaultFont;
    }
    return ff.toString();
}

QFont Appearance::font(){
    return QFont(fontFamily(), fontSize());
}

QString Appearance::theme(){
    auto cm = ConfigManager::instance();
    auto th = cm->get(CONFIG_NAME, KEY_THEME);
    if(th.isNull()){
        // 使用系统默认样式，在Windows上通常是"Windows"
        QString defaultStyle = QApplication::style()->objectName();
        if (defaultStyle.isEmpty()) {
            defaultStyle = "Windows"; // Windows平台的默认样式
        }
        cm->set(CONFIG_NAME, KEY_THEME, defaultStyle);
        return defaultStyle;
    }
    return th.toString();
}

QString Appearance::colorTheme(){
    auto cm = ConfigManager::instance();
    auto ct = cm->get(CONFIG_NAME, KEY_COLOR_THEME);
    if(ct.isNull()){
        cm->set(CONFIG_NAME, KEY_COLOR_THEME, "Light");
        return "Light";
    }
    return ct.toString();
}

QString Appearance::language(){
    auto cm = ConfigManager::instance();
    auto lang = cm->get(CONFIG_NAME, KEY_LANGUAGE);
    if(lang.isNull()){
        cm->set(CONFIG_NAME, KEY_LANGUAGE, "en");
        return "en";
    }
    return lang.toString();
}

void Appearance::setFontSize(int size){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_FONT_SIZE, size);
    
    emit instance()->fontChanged(font());
}

void Appearance::setFontFamily(const QString& family){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_FONT_FAMILY, family);
    
    emit instance()->fontChanged(font());
}

void Appearance::setTheme(const QString& theme){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_THEME, theme);
    
    emit instance()->themeChanged(theme);
}

void Appearance::setColorTheme(const QString& colorTheme){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_COLOR_THEME, colorTheme);
    
    emit instance()->colorThemeChanged(colorTheme);
}

void Appearance::setLanguage(const QString& language){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_LANGUAGE, language);
    
    emit instance()->languageChanged(language);
}

void Appearance::setupApp(){
    auto fontObj = font();
    auto themeStr = theme();
    auto colorThemeStr = colorTheme();

    LOG_INFO(QString("Setting up application with font: %1 %2pt, theme: %3, color theme: %4")
             .arg(fontObj.family()).arg(fontObj.pointSize()).arg(themeStr, colorThemeStr));

    // 首先设置应用程序样式
    QStringList styles = QStyleFactory::keys();
    LOG_INFO(QString("Available styles: %1").arg(styles.join(", ")));
    
    if(styles.contains(themeStr, Qt::CaseInsensitive)){
        QApplication::setStyle(QStyleFactory::create(themeStr));
        LOG_INFO(QString("Style set to: %1").arg(themeStr));
    } else {
        LOG_WARNING(QString("Style '%1' not found, using default").arg(themeStr));
    }
    
    // 设置颜色方案
    if(colorThemeStr == "System"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
    }else if(colorThemeStr == "Light"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }else if(colorThemeStr == "Dark"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    }
    
    // 最后设置字体（在样式设置之后，确保字体不被样式覆盖）
    QApplication::setFont(fontObj);
    LOG_INFO(QString("Application font set to: %1 %2pt")
             .arg(QApplication::font().family()).arg(QApplication::font().pointSize()));
}

} // namespace Config
