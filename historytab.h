#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

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
private:
    Ui::HistoryTab *ui;

    HistoryTableModel* m_model;
};

#endif // HISTORYTAB_H
