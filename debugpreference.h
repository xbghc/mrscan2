#ifndef DEBUGPREFERENCE_H
#define DEBUGPREFERENCE_H

#include "ipreferencewidget.h"

namespace Ui {
class DebugPreference;
}

class DebugPreference : public IPreferenceWidget
{
    Q_OBJECT

public:
    explicit DebugPreference(QWidget *parent = nullptr);
    ~DebugPreference() override;
    
    void load() override;
    void save() override;

private slots:
    void onBrowseButtonClicked();
    void onBrowseLogPathButtonClicked();
    void onMockFilePathChanged();
    void onScanTimeChanged();
    void onEnableDelayChanged();
    void onLogLevelChanged();
    void onLogToFileChanged();
    void onLogFilePathChanged();

private:
    std::unique_ptr<Ui::DebugPreference> ui;
};

#endif // DEBUGPREFERENCE_H
