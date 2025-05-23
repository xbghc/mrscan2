#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>

#include "exam.h"
#include "historymodel.h"

namespace Ui {
class HistoryTab;
}

class HistoryTab : public QWidget {
    Q_OBJECT

public:
    explicit HistoryTab(QWidget *parent = nullptr);
    ~HistoryTab();

    void loadHistoryList();
    void addExamToView(const Exam& exam);

signals:
    void currentItemChanged(const Exam &exam);

private slots:
    void onCurrentRowChanged();
    
private:
    Ui::HistoryTab *ui;

    HistoryModel *m_model;
    std::map<std::pair<QString, QString>, Exam> m_cache;
    
    void setupConnections(); // 设置信号连接
};

#endif // HISTORYTAB_H
