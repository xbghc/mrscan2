#ifndef QCHECKCOMBOBOX_H
#define QCHECKCOMBOBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QPropertyAnimation>
#include <memory>

/**
 * @brief Multi-selection combo box widget with checkable items
 * 
 * Provides a dropdown list where users can select multiple items using checkboxes.
 * Selected items are displayed in a read-only line edit with customizable separator.
 */
class QCheckComboBox : public QWidget
{
    Q_OBJECT

public:
    enum Filter { ALL, CHECKED, UNCHECKED };

    explicit QCheckComboBox(QWidget *parent = nullptr);
    ~QCheckComboBox();

    // Item management
    void addItem(const QString& text, const QVariant& data = QVariant());
    void setItems(const QStringList& items);
    void removeAllItems();
    int itemCount() const;

    // Selection management
    void setChecked(int index, bool checked);
    bool isChecked(int index) const;
    void selectAll();
    void deselectAll();
    void setCheckedByTexts(const QStringList& texts);

    // Data retrieval
    QList<QVariant> values(Filter filter = ALL);
    QStringList checkedTexts() const;
    QList<int> checkedIndexes() const;

    // Appearance customization
    void setButtonIcon(const QIcon& icon);
    void setSeparator(const QString& separator);
    void setPlaceholderText(const QString& placeholder);

    // Size hints
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void itemStatusChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void onItemClicked(const QModelIndex& index);

private:
    // UI Constants
    static constexpr QSize BUTTON_SIZE{24, 24};
    static constexpr int ITEM_HEIGHT = 32;
    static constexpr int MIN_POPUP_HEIGHT = 100;
    static constexpr int MAX_POPUP_HEIGHT = 200;
    static constexpr int ANIMATION_DURATION = 150;
    static constexpr int PADDING_WIDTH = 20;
    static constexpr int MIN_WIDTH = 80;
    static constexpr int MAX_DISPLAY_ITEMS = 3;
    
    // Style sheets
    static const QString BUTTON_STYLE;
    static const QString TEXT_STYLE;
    static const QString POPUP_STYLE;

    // UI components
    std::unique_ptr<QLineEdit> m_textEdit;
    std::unique_ptr<QPushButton> m_dropButton;
    std::unique_ptr<QListView> m_popup;
    std::unique_ptr<QStandardItemModel> m_model;
    std::unique_ptr<QPropertyAnimation> m_animation;

    // Configuration
    QString m_separator{", "};
    QString m_placeholderText;
    mutable QSize m_cachedSize;

    // Core functionality
    void setupUI();
    void setupAnimation();
    void updateLayout();
    void updateDisplayText();
    void notifyDataChanged();

    // Popup management
    void showPopup();
    void hidePopup();
    void togglePopup();
    QRect calculatePopupGeometry() const;

    // Helper methods
    QString calculateDisplayText() const;
    QSize calculateSize(bool preferredSize) const;
    bool isValidIndex(int index) const;
    bool isItemChecked(int index) const;
    void setAllChecked(bool checked);
    QStandardItem* createItem(const QString& text, const QVariant& data = QVariant()) const;
    
    // Event handling helpers
    bool shouldHideOnFocusOut(QFocusEvent* event) const;
    bool isWidgetPartOfComboBox(QWidget* widget) const;
};

#endif // QCHECKCOMBOBOX_H
