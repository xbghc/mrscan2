#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

#include "examhistory.h"
#include "historymodel.h"

namespace Ui {
class HistoryTab;
}

class HistoryTab : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTab(QWidget *parent = nullptr);
    ~HistoryTab();

    void loadHistoryList();

signals:
    void currentIndexChanged(ExamHistory history);
private:
    Ui::HistoryTab *ui;

    HistoryModel* m_model;
};

#endif // HISTORYTAB_H
