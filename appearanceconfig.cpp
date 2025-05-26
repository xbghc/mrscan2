#include "appearanceconfig.h"
#include "configmanager.h"
#include "utils.h"

#include <QApplication>
#include <QGuiApplication>
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
        cm->set(CONFIG_NAME, KEY_FONT_FAMILY, "Arial");
        return "Arial";
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
        cm->set(CONFIG_NAME, KEY_THEME, "Light");
        return "Light";
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

void Appearance::setFontSize(int size){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_FONT_SIZE, size);
    
    // 直接发射信号
    emit instance()->fontChanged(font());
}

void Appearance::setFontFamily(const QString& family){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_FONT_FAMILY, family);
    
    // 直接发射信号
    emit instance()->fontChanged(font());
}

void Appearance::setTheme(const QString& theme){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_THEME, theme);
    
    // 直接发射信号
    emit instance()->themeChanged(theme);
}

void Appearance::setColorTheme(const QString& colorTheme){
    auto cm = ConfigManager::instance();
    cm->set(CONFIG_NAME, KEY_COLOR_THEME, colorTheme);
    
    // 直接发射信号
    emit instance()->colorThemeChanged(colorTheme);
}

void Appearance::setupApp(){
    auto fontObj = font();
    auto themeStr = theme();
    auto colorThemeStr = colorTheme();

    LOG_INFO(QString("Setting up application with font: %1, theme: %2, color theme: %3")
             .arg(fontObj.family(), themeStr, colorThemeStr));

    QApplication::setFont(fontObj);
    
    // 设置应用程序样式
    QStringList styles = QStyleFactory::keys();
    if(styles.contains(themeStr, Qt::CaseInsensitive)){
        QApplication::setStyle(QStyleFactory::create(themeStr));
    }
    
    // 设置颜色方案
    if(colorThemeStr == "System"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
    }else if(colorThemeStr == "Light"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }else if(colorThemeStr == "Dark"){
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    }
}

} // namespace Config
