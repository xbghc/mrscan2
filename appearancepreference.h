#ifndef APPEARANCEPREFERENCE_H
#define APPEARANCEPREFERENCE_H

#include <QWidget>
#include "ipreferencewidget.h"

namespace Ui {
class AppearancePreference;
}

class AppearancePreference : public IPreferenceWidget
{
    Q_OBJECT

public:
    explicit AppearancePreference(QWidget *parent = nullptr);
    ~AppearancePreference() override;

    void save() override;
    void load() override;

private slots:
    void updateFontPreview();

private:
    std::unique_ptr<Ui::AppearancePreference> ui;
    void setupConnections();
    void setupStyleComboBox();
    void setupFontComboBox();
};

#endif // APPEARANCEPREFERENCE_H
