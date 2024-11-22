#include "custompreferences.h"

#include <QFile>
#include <QFont>
#include <QJsonDocument>
#include <QStyleFactory>

const QString CustomPreferences::filePath = "./configs/preferences.json";

CustomPreferences::CustomPreferences() {}

QJsonObject CustomPreferences::load()
{
    QFile file(filePath);
    if(!file.exists() || !file.open(QIODevice::ReadOnly)){
        qDebug() << "can't open file: " << filePath;
        return QJsonObject();
    }

    return QJsonDocument::fromJson(file.readAll()).object();
}

void CustomPreferences::save(QJsonObject p)
{
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "cant open file: " << filePath;
    }

    file.write(QJsonDocument(p).toJson());
}

void CustomPreferences::setupApp()
{
    auto preferences = load();

    setupApp(preferences);
}

void CustomPreferences::setupApp(QJsonObject preferences)
{

    setupAppStyle(preferences["style"].toObject());
}

void CustomPreferences::setupAppStyle(QJsonObject style)
{   
    if(style.contains("fontFamily") && style.contains("fontSize")){
        auto fontFamily = style["fontFamily"].toString();
        auto fontSize = style["fontSize"].toInt();
        auto font = QFont(fontFamily, fontSize);
        QApplication::setFont(font);
    }

    if(style.contains("theme")){
        auto theme = style["theme"].toString();
        QStringList styles = QStyleFactory::keys();

        if(styles.contains(theme, Qt::CaseInsensitive)){
            QApplication::setStyle(QStyleFactory::create(theme));
        }else{
            qDebug() << "unkown theme: " << theme;
        }
    }
}
