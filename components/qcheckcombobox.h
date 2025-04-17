#ifndef QCHECKCOMBOBOX_H
#define QCHECKCOMBOBOX_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QPropertyAnimation>

class QCheckComboBox : public QWidget
{
    Q_OBJECT

public:
    enum Filter{ALL, CHECKED, UNCHECKED};

    explicit QCheckComboBox(QWidget *parent = nullptr);
    ~QCheckComboBox();

    void addItem(const QString& text, const QVariant& data=QVariant());
    QList<QVariant> values(QCheckComboBox::Filter filter=Filter::ALL); // Using ALL requires custom filter instead of Qt::Status

    int itemCount() const;
    void setChecked(int index, bool checked);
    void removeAllItems();
    bool isChecked(int index) const;

signals:
    void itemStatusChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onItemClicked(const QModelIndex&);

private:
    QTextEdit* m_text;
    QPushButton* m_button;
    QListView* m_popup;
    QStandardItemModel* m_model;

    void updateLayout();
    void showPopup();
    void hidePopup();

    constexpr static QSize BUTTON_SIZE = QSize(24, 24);

    void initButton();
    void initPopup();

    void passButtonClick(QObject *obj, QEvent *event);
};

#endif // QCHECKCOMBOBOX_H
