#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

#include "historymodel.h"

namespace Ui {
class HistoryTab;
}

/// @todo 应该向外传递selection改变的信号
class HistoryTab : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTab(QWidget *parent = nullptr);
    ~HistoryTab();

    void loadHistoryList();

private:
    Ui::HistoryTab *ui;

    HistoryModel* m_model;
};

#endif // HISTORYTAB_H
