#ifndef EXAMTAB_H
#define EXAMTAB_H

#include <QWidget>

namespace Ui {
class studytab;
}

class ExamTab : public QWidget
{
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();

public slots:
    void loadPatients();

private slots:
    void on_toolButton_3_clicked();

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

private:
    Ui::studytab *ui;
};

#endif // EXAMTAB_H
