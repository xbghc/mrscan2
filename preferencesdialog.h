#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <memory>

#include "ipreferencewidget.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    /**
     * @brief 添加配置页面
     * @param label 左侧列表的名称
     * @param widget 标签页的指针
     * @return 标签页的索引
     */
    int addWidget(const QString& label, IPreferenceWidget* widget);

private slots:
    void onApplyButtonClicked();
private:
    std::unique_ptr<Ui::PreferencesDialog> ui;

    void setupConnections();
};

#endif // PREFERENCESDIALOG_H
