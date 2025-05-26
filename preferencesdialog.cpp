#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "appearancepreference.h"
#include "debugpreference.h"
#include "utils.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    
    setupConnections();

    // 添加外观配置页面
    auto appearanceWidget = new AppearancePreference(this);
    addWidget("外观", appearanceWidget);

    // 添加调试配置页面
    auto debugWidget = new DebugPreference(this);
    addWidget("调试", debugWidget);
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::setupConnections(){
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, onApplyButtonClicked);
    connect(ui->listWidget, &QListWidget::currentRowChanged, 
            ui->stackedWidget, &QStackedWidget::setCurrentIndex);
}

void PreferencesDialog::onApplyButtonClicked(){
    for(auto widget:ui->stackedWidget->children()){
        IPreferenceWidget* preferenceWidget = qobject_cast<IPreferenceWidget*>(widget);
        if(preferenceWidget){
            preferenceWidget->save();
        }
    }
}

int PreferencesDialog::addWidget(const QString& label, IPreferenceWidget* widget)
{
    // 添加到左侧列表
    ui->listWidget->addItem(label);
    
    // 添加到右侧堆叠窗口
    widget->load();
    int index = ui->stackedWidget->addWidget(widget);
    
    // 如果是第一个窗口，设置为当前选中
    if(ui->listWidget->count() == 1) {
        ui->listWidget->setCurrentRow(0);
        ui->stackedWidget->setCurrentIndex(index);
    }
    
    return index;
}
