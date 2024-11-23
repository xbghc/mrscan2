#include "preferencestyle.h"
#include "ui_preferencestyle.h"

#include <QFontDatabase>
#include <QStringListModel>
#include <QStyleFactory>


PreferenceStyle::PreferenceStyle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferenceStyle)
{
    ui->setupUi(this);

    QStringList families = QFontDatabase::families();
    QStringListModel *model = new QStringListModel(families);
    ui->inputFontFamily->setModel(model);

    QStringList themes = QStyleFactory::keys();
    ui->inputTheme->setModel(new QStringListModel(themes));

}

PreferenceStyle::~PreferenceStyle()
{
    delete ui;
}

void PreferenceStyle::set(QJsonObject style)
{
    auto font = QApplication::font();

    setFontFamily(font.family());

    setFontSize(font.pointSize());

    setTheme(QApplication::style()->name());

    setColorScheme(QGuiApplication::styleHints()->colorScheme());
}

QJsonObject PreferenceStyle::get()
{
    QJsonObject style;
    style["fontSize"] = ui->inputFontSize->value();
    style["fontFamily"] = ui->inputFontFamily->currentText();
    style["theme"] = ui->inputTheme->currentText();
    style["colorScheme"] = ui->inputColorScheme->currentText();

    return style;
}

void PreferenceStyle::setFontSize(int size)
{
    ui->inputFontSize->setValue(size);
}

void PreferenceStyle::setFontFamily(QString fontFamily)
{
    int fontFamilyIndex = ui->inputFontFamily->findText(fontFamily);
    if(fontFamilyIndex >= 0){
        ui->inputFontFamily->setCurrentIndex(fontFamilyIndex);
    }else{
        qDebug() << "unknown font family: " << fontFamily;
    }
}

void PreferenceStyle::setTheme(QString theme)
{
    int index = ui->inputTheme->findText(theme, Qt::MatchFixedString);
    if(index >= 0){
        ui->inputTheme->setCurrentIndex(index);
    }else{
        qDebug() << "unknown theme: " << theme;
    }
}

void PreferenceStyle::setColorScheme(Qt::ColorScheme color)
{
    if(color == Qt::ColorScheme::Unknown){
        ui->inputColorScheme->setCurrentIndex(0);
        return;
    }

    if(color == Qt::ColorScheme::Light){
        ui->inputColorScheme->setCurrentIndex(1);
        return;
    }

    if(color == Qt::ColorScheme::Dark){
        ui->inputColorScheme->setCurrentIndex(2);
        return;
    }
}
