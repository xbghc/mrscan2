#ifndef APPEARANCECONFIG_H
#define APPEARANCECONFIG_H

#include <QString>
#include <QFont>
#include <QObject>

namespace config{
    class Appearance : public QObject{
        Q_OBJECT
        
    public:
        static constexpr const char* CONFIG_NAME = "Appearance";
        static constexpr const char* KEY_FONT_SIZE = "font_size";
        static constexpr const char* KEY_FONT_FAMILY = "font_family";
        static constexpr const char* KEY_THEME = "theme";
        static constexpr const char* KEY_COLOR_THEME = "color_theme";
        static constexpr const char* KEY_LANGUAGE = "language";

        // 单例实例
        static Appearance* instance();
        
        // 静态接口（直接实现，不需要Impl）
        static int fontSize();
        static QString fontFamily();
        static QFont font();
        static QString theme();
        static QString colorTheme();
        static QString language();

        static void setFontSize(int size);
        static void setFontFamily(const QString& family);
        static void setTheme(const QString& theme);
        static void setColorTheme(const QString& colorTheme);
        static void setLanguage(const QString& language);

        static void setupApp();
        
    signals:
        void fontChanged(const QFont& newFont);
        void themeChanged(const QString& newTheme);
        void colorThemeChanged(const QString& newColorTheme);
        void languageChanged(const QString& newLanguage);
        
    private:
        explicit Appearance(QObject *parent = nullptr);
    };
}

#endif // APPEARANCECONFIG_H
