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

private:
    Ui::studytab *ui;
};

#endif // EXAMTAB_H
