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
    setupFontComboBox();
}

AppearancePreference::~AppearancePreference()
{
}

void AppearancePreference::setupConnections()
{
    // 字体设置变化时更新预览
    connect(ui->fontFamilyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
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

void AppearancePreference::setupFontComboBox()
{
    // 填充系统可用字体
    QFontDatabase fontDatabase;
    QStringList fontFamilies = fontDatabase.families();
    ui->fontFamilyComboBox->addItems(fontFamilies);
}

void AppearancePreference::updateFontPreview()
{
    QString fontFamily = ui->fontFamilyComboBox->currentText();
    int fontSize = ui->fontSizeSpinBox->value();
    QFont previewFont(fontFamily, fontSize);
    ui->fontPreview->setFont(previewFont);
}

void AppearancePreference::save()
{
    // 保存字体设置
    config::Appearance::setFontFamily(ui->fontFamilyComboBox->currentText());
    config::Appearance::setFontSize(ui->fontSizeSpinBox->value());
    
    // 保存主题设置
    config::Appearance::setTheme(ui->styleComboBox->currentText());
    config::Appearance::setColorTheme(ui->colorThemeComboBox->currentText());
    
    // 保存语言设置 - 使用语言代码而不是显示文本
    QString languageCode;
    int currentIndex = ui->languageComboBox->currentIndex();
    if (currentIndex == 0) {
        languageCode = "en";  // English
    } else if (currentIndex == 1) {
        languageCode = "zh";  // Chinese
    } else {
        languageCode = "en";  // 默认英文
    }
    config::Appearance::setLanguage(languageCode);

    config::Appearance::setupApp();
}

void AppearancePreference::load()
{
    // 加载字体设置
    QString fontFamily = config::Appearance::fontFamily();
    int fontSize = config::Appearance::fontSize();
    
    int fontIndex = ui->fontFamilyComboBox->findText(fontFamily);
    if (fontIndex != -1) {
        ui->fontFamilyComboBox->setCurrentIndex(fontIndex);
    }
    ui->fontSizeSpinBox->setValue(fontSize);
    
    // 加载主题设置
    QString theme = config::Appearance::theme();
    QString colorTheme = config::Appearance::colorTheme();
    QString languageCode = config::Appearance::language();
    
    int themeIndex = ui->styleComboBox->findText(theme);
    if(themeIndex != -1) {
        ui->styleComboBox->setCurrentIndex(themeIndex);
    }
    
    int colorThemeIndex = ui->colorThemeComboBox->findText(colorTheme);
    if(colorThemeIndex != -1) {
        ui->colorThemeComboBox->setCurrentIndex(colorThemeIndex);
    }
    
    // 加载语言设置 - 根据语言代码设置下拉框
    int languageIndex = 0;  // 默认英文
    if (languageCode == "en" || languageCode == "English") {
        languageIndex = 0;
    } else if (languageCode == "zh" || languageCode == "Chinese") {
        languageIndex = 1;
    }
    ui->languageComboBox->setCurrentIndex(languageIndex);
    
    // 更新字体预览
    updateFontPreview();
}
