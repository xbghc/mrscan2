#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

#include "examhistory.h"
#include "historytablemodel.h"

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
    void currentHistoryChanged(ExamHistory history);
private:
    Ui::HistoryTab *ui;

    HistoryTableModel* m_model;
};

#endif // HISTORYTAB_H
