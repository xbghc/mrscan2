#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <memory>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

private slots:
    void on_listWidget_currentRowChanged(int currentRow);

    void on_pushButton_clicked();

private:
    std::unique_ptr<Ui::PreferencesDialog> ui;
    void set(QJsonObject preferences);
    QJsonObject get();
};

#endif // PREFERENCESDIALOG_H
