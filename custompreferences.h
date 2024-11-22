#ifndef CUSTOMPREFERENCES_H
#define CUSTOMPREFERENCES_H

#include <QApplication>
#include <QJsonObject>

class CustomPreferences
{
public:
    CustomPreferences();

    static QJsonObject load();
    static void save(QJsonObject p);
    static void setupApp();
    static void setupApp(QJsonObject preferences);
    static void setupAppStyle(QJsonObject style);

private:
    const static QString filePath;
};

#endif // CUSTOMPREFERENCES_H
