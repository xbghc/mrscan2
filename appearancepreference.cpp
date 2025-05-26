#include "appearancepreference.h"
#include "ui_appearancepreference.h"
#include "appearanceconfig.h"
#include <QStyleFactory>
#include <QFontDatabase>

AppearancePreference::AppearancePreference(QWidget *parent)
    : IPreferenceWidget(parent)
    , ui(new Ui::AppearancePreference)
{
    ui->setupUi(this);
    
    setupConnections();
    setupStyleComboBox();
}

AppearancePreference::~AppearancePreference()
{
}

void AppearancePreference::setupConnections()
{
    // 字体设置变化时更新预览
    connect(ui->fontFamilyComboBox, &QFontComboBox::currentFontChanged,
            this, &AppearancePreference::updateFontPreview);
    connect(ui->fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AppearancePreference::updateFontPreview);
}

void AppearancePreference::setupStyleComboBox()
{
    // 填充可用的应用程序样式
    QStringList styles = QStyleFactory::keys();
    ui->styleComboBox->addItems(styles);
}

void AppearancePreference::updateFontPreview()
{
    QFont previewFont = ui->fontFamilyComboBox->currentFont();
    previewFont.setPointSize(ui->fontSizeSpinBox->value());
    ui->fontPreview->setFont(previewFont);
}

void AppearancePreference::save()
{
    // 保存字体设置
    config::Appearance::setFontFamily(ui->fontFamilyComboBox->currentFont().family());
    config::Appearance::setFontSize(ui->fontSizeSpinBox->value());
    
    // 保存主题设置
    config::Appearance::setTheme(ui->styleComboBox->currentText());
    config::Appearance::setColorTheme(ui->colorThemeComboBox->currentText());
    
    // 保存语言设置
    config::Appearance::setLanguage(ui->languageComboBox->currentText());

    config::Appearance::setupApp();
}

void AppearancePreference::load()
{
    // 加载字体设置
    QString fontFamily = config::Appearance::fontFamily();
    int fontSize = config::Appearance::fontSize();
    
    ui->fontFamilyComboBox->setCurrentFont(QFont(fontFamily));
    ui->fontSizeSpinBox->setValue(fontSize);
    
    // 加载主题设置
    QString theme = config::Appearance::theme();
    QString colorTheme = config::Appearance::colorTheme();
    QString language = config::Appearance::language();
    
    int themeIndex = ui->styleComboBox->findText(theme);
    if(themeIndex != -1) {
        ui->styleComboBox->setCurrentIndex(themeIndex);
    }
    
    int colorThemeIndex = ui->colorThemeComboBox->findText(colorTheme);
    if(colorThemeIndex != -1) {
        ui->colorThemeComboBox->setCurrentIndex(colorThemeIndex);
    }
    
    int languageIndex = ui->languageComboBox->findText(language);
    if(languageIndex != -1) {
        ui->languageComboBox->setCurrentIndex(languageIndex);
    }
    
    // 更新字体预览
    updateFontPreview();
}
