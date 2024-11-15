#ifndef EXAMINFODIALOG_H
#define EXAMINFODIALOG_H

#include <QDialog>

namespace Ui {
class ExamInfoDialog;
}

class ExamInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExamInfoDialog(QWidget *parent = nullptr);
    ~ExamInfoDialog();

private:
    Ui::ExamInfoDialog *ui;
};

#endif // EXAMINFODIALOG_H
