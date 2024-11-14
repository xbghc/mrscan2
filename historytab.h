#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

namespace Ui {
class HistoryTab;
}

class HistoryTab : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTab(QWidget *parent = nullptr);
    ~HistoryTab();

private:
    Ui::HistoryTab *ui;
};

#endif // HISTORYTAB_H
