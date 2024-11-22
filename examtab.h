#ifndef EXAMTAB_H
#define EXAMTAB_H

#include <QWidget>

#include "examhistory.h"
#include "examtablemodel.h"
#include "scanneradapter.h"

namespace Ui {
class studytab;
}

class ExamTab : public QWidget
{
    Q_OBJECT

public:
    explicit ExamTab(QWidget *parent = nullptr);
    ~ExamTab();
    int currentExamIndex();
signals:
    void currentExamChanged(QJsonObject patient, QJsonObject exam);
    void displayExam(ExamHistory history);
    void scanned();
public slots:
    void loadPatients();

private slots:
    void on_toolButton_3_clicked();

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_2_clicked();

    void on_scanButton_clicked();

private:
    Ui::studytab *ui;
    ExamTableModel* examModel;
    ScannerAdapter* adapter;
    QString getStatus(int row);
};

#endif // EXAMTAB_H
