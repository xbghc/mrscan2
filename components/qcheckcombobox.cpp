/**
 * @file qcheckcombobox.cpp
 * @brief Implementation of QCheckComboBox - A multi-selection combo box widget
 * 
 * This file contains the implementation of QCheckComboBox, which provides
 * a dropdown list with checkable items for multi-selection functionality.
 */

#include "qcheckcombobox.h"

#include <QSizePolicy>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QApplication>
#include <QFontMetrics>
#include <QObject>

// Style definitions
const QString QCheckComboBox::BUTTON_STYLE = R"(
QPushButton {
    border: none;
    background: transparent;
    border-radius: 3px;
}
QPushButton:hover {
    background: rgba(0, 0, 0, 0.1);
}
QPushButton:pressed {
    background: rgba(0, 0, 0, 0.2);
}
)";

const QString QCheckComboBox::TEXT_STYLE = R"(
QLineEdit {
    border: 1px solid #d0d0d0;
    border-radius: 4px;
    padding: 4px 8px;
    background: white;
    selection-background-color: #3daee9;
}
QLineEdit:focus {
    border-color: #3daee9;
    outline: none;
}
QLineEdit:disabled {
    background: #f5f5f5;
    color: #666666;
}
)";

const QString QCheckComboBox::POPUP_STYLE = R"(
QListView {
    border: 1px solid #c0c0c0;
    border-radius: 4px;
    background: white;
    outline: none;
    selection-background-color: #e6f3ff;
}
QListView::item {
    padding: 4px 8px;
    border: none;
    height: %1px;
}
QListView::item:hover {
    background: #f0f8ff;
}
QListView::item:selected {
    color: #000000;
    background: #e6f3ff;
}
)";

QCheckComboBox::QCheckComboBox(QWidget *parent)
    : QWidget(parent)
    , m_textEdit(std::make_unique<QLineEdit>(this))
    , m_dropButton(std::make_unique<QPushButton>(this))
    , m_popup(std::make_unique<QListView>(this))
    , m_model(std::make_unique<QStandardItemModel>(this))
    , m_animation(std::make_unique<QPropertyAnimation>(this))
    , m_separator(", "),
    m_placeholderText(""),
    m_cachedSize(0, 0)
{
    setupUI();
    setupAnimation();
    updateLayout();
    setFocusPolicy(Qt::StrongFocus);
}

QCheckComboBox::~QCheckComboBox()
{
    if (m_popup && m_popup->isVisible()) {
        QApplication::instance()->removeEventFilter(this);
    }
}

// ============================================================================
// Public Interface - Item Management
// ============================================================================

void QCheckComboBox::addItem(const QString &text, const QVariant &data)
{
    if (text.isEmpty()) {
        qWarning() << "QCheckComboBox: Adding item with empty text";
        return;
    }

    auto item = createItem(text, data);
    m_model->appendRow(item);
    notifyDataChanged();
}

void QCheckComboBox::setItems(const QStringList &items)
{
    m_model->clear();
    for (const QString &text : items) {
        if (!text.isEmpty()) {
            m_model->appendRow(createItem(text));
        }
    }
    notifyDataChanged();
}

void QCheckComboBox::removeAllItems()
{
    m_model->clear();
    notifyDataChanged();
}

int QCheckComboBox::itemCount() const
{
    return m_model->rowCount();
}

// ============================================================================
// Public Interface - Selection Management
// ============================================================================

void QCheckComboBox::setChecked(int index, bool checked)
{
    if (!isValidIndex(index)) {
        return;
    }
    
    QModelIndex modelIndex = m_model->index(index, 0);
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    m_model->setData(modelIndex, state, Qt::CheckStateRole);
    notifyDataChanged();
}

bool QCheckComboBox::isChecked(int index) const
{
    return isItemChecked(index);
}

void QCheckComboBox::selectAll()
{
    setAllChecked(true);
}

void QCheckComboBox::deselectAll()
{
    setAllChecked(false);
}

void QCheckComboBox::setCheckedByTexts(const QStringList &texts)
{
    setAllChecked(false);
    
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        QString itemText = m_model->data(index, Qt::DisplayRole).toString();
        if (texts.contains(itemText)) {
            m_model->setData(index, Qt::Checked, Qt::CheckStateRole);
        }
    }
    notifyDataChanged();
}

// ============================================================================
// Public Interface - Data Retrieval
// ============================================================================

QList<QVariant> QCheckComboBox::values(QCheckComboBox::Filter filter)
{
    QList<QVariant> result;
    
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        bool isChecked = m_model->data(index, Qt::CheckStateRole).toInt() == Qt::Checked;
        
        bool shouldInclude = (filter == ALL) || 
                           (filter == CHECKED && isChecked) || 
                           (filter == UNCHECKED && !isChecked);
        
        if (shouldInclude) {
            QVariant value = m_model->data(index, Qt::UserRole);
            result.append(value.isNull() ? m_model->data(index, Qt::DisplayRole) : value);
        }
    }
    
    return result;
}

QStringList QCheckComboBox::checkedTexts() const
{
    QStringList result;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (isItemChecked(i)) {
            QModelIndex index = m_model->index(i, 0);
            result << m_model->data(index, Qt::DisplayRole).toString();
        }
    }
    return result;
}

QList<int> QCheckComboBox::checkedIndexes() const
{
    QList<int> result;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (isItemChecked(i)) {
            result << i;
        }
    }
    return result;
}

// ============================================================================
// Public Interface - Appearance Customization
// ============================================================================

void QCheckComboBox::setButtonIcon(const QIcon &icon)
{
    m_dropButton->setIcon(icon);
}

void QCheckComboBox::setSeparator(const QString &separator)
{
    if (separator.isEmpty()) {
        qWarning() << "QCheckComboBox: Empty separator provided";
        return;
    }
    m_separator = separator;
    updateDisplayText();
}

void QCheckComboBox::setPlaceholderText(const QString &placeholder)
{
    m_placeholderText = placeholder;
    m_textEdit->setPlaceholderText(placeholder);
    updateDisplayText();
}

// ============================================================================
// Public Interface - Size Hints
// ============================================================================

QSize QCheckComboBox::sizeHint() const
{
    return calculateSize(true);
}

QSize QCheckComboBox::minimumSizeHint() const
{
    return calculateSize(false);
}

// ============================================================================
// Protected Event Handlers
// ============================================================================

void QCheckComboBox::resizeEvent(QResizeEvent *event)
{
    updateLayout();
    
    // Update popup width if visible
    if (m_popup->isVisible()) {
        QRect popupGeometry = m_popup->geometry();
        popupGeometry.setWidth(width());
        m_popup->setGeometry(popupGeometry);
    }
    
    QWidget::resizeEvent(event);
}

void QCheckComboBox::mousePressEvent(QMouseEvent *event)
{
    togglePopup();
    event->accept();
}

bool QCheckComboBox::eventFilter(QObject *obj, QEvent *event)
{
    // Handle button click
    if (obj == m_dropButton.get() && event->type() == QEvent::MouseButtonPress) {
        togglePopup();
        return true;
    }
    
    // Handle events when popup is visible
    if (m_popup && m_popup->isVisible()) {
        switch (event->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            // Only hide popup if focus moves outside our component
            if (!isWidgetPartOfComboBox(QApplication::focusWidget())) {
                hidePopup();
            }
            return false; // Don't consume focus events
            
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            // Hide popup when window activation changes
            hidePopup();
            return false; // Don't consume window events
            
        case QEvent::Move:
        case QEvent::Resize:
            // Hide popup when parent window moves or resizes
            if (obj && obj->isWidgetType()) {
                QWidget* widget = static_cast<QWidget*>(obj);
                if (widget->isWindow()) {
                    QWidget* topLevel = this->window();
                    if (widget == topLevel) {
                        hidePopup();
                    }
                }
            }
            return false; // Don't consume move/resize events
            
        case QEvent::MouseButtonPress: {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = mouseEvent->globalPosition().toPoint();
            
            // Check if click is inside popup
            if (m_popup->geometry().contains(globalPos)) {
                return false; // Let popup handle the click
            }
            
            // Click is outside - hide popup but don't consume the event
            hidePopup();
            return false; // Let the event propagate to other widgets
        }
        default:
            break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void QCheckComboBox::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        togglePopup();
        event->accept();
        break;
        
    case Qt::Key_Escape:
        if (m_popup->isVisible()) {
            hidePopup();
            event->accept();
        } else {
            QWidget::keyPressEvent(event);
        }
        break;

    case Qt::Key_A:
        if (event->modifiers() & Qt::ControlModifier) {
            selectAll();
            event->accept();
        } else {
            QWidget::keyPressEvent(event);
        }
        break;
        
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void QCheckComboBox::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    update();
}

void QCheckComboBox::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    if (m_popup->isVisible() && shouldHideOnFocusOut(event)) {
        hidePopup();
    }
    update();
}

// ============================================================================
// Private Slots
// ============================================================================

void QCheckComboBox::onItemClicked(const QModelIndex& index)
{
    Qt::CheckState currentState = static_cast<Qt::CheckState>(
        m_model->data(index, Qt::CheckStateRole).toInt());
    Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    
    m_model->setData(index, newState, Qt::CheckStateRole);
    notifyDataChanged();
}

// ============================================================================
// Private Implementation - Core Functionality
// ============================================================================

void QCheckComboBox::setupUI()
{
    // Setup text edit
    m_textEdit->setStyleSheet(TEXT_STYLE);
    m_textEdit->setReadOnly(true);
    m_textEdit->setPlaceholderText(m_placeholderText);

    // Setup button
    m_dropButton->setStyleSheet(BUTTON_STYLE);
    m_dropButton->setFixedSize(BUTTON_SIZE);
    m_dropButton->setIcon(QIcon(":/qcheckcombobox.svg"));
    m_dropButton->setIconSize(QSize(16, 16));
    m_dropButton->installEventFilter(this);

    // Setup popup
    m_popup->hide();
    m_popup->setModel(m_model.get());
    m_popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_popup->setUniformItemSizes(true);
    m_popup->setStyleSheet(POPUP_STYLE.arg(ITEM_HEIGHT));
    m_popup->installEventFilter(this);
    
    connect(m_popup.get(), &QListView::clicked, this, &QCheckComboBox::onItemClicked);
}

void QCheckComboBox::setupAnimation()
{
    m_animation->setTargetObject(m_popup.get());
    m_animation->setPropertyName("geometry");
    m_animation->setDuration(ANIMATION_DURATION);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void QCheckComboBox::updateLayout()
{
    auto margins = contentsMargins();
    int availableWidth = width() - margins.left() - margins.right();
    int availableHeight = height() - margins.top() - margins.bottom();
    
    // Calculate text edit geometry
    int buttonSpace = BUTTON_SIZE.width() + 8;
    int textWidth = availableWidth - buttonSpace;
    
    QFontMetrics fm(font());
    int textHeight = fm.height() + 8;
    int textY = margins.top() + (availableHeight - textHeight) / 2;
    
    m_textEdit->setGeometry(margins.left(), textY, textWidth, textHeight);
    
    // Calculate button geometry
    int buttonX = width() - BUTTON_SIZE.width() - margins.right() - 4;
    int buttonY = margins.top() + (availableHeight - BUTTON_SIZE.height()) / 2;
    m_dropButton->setGeometry(buttonX, buttonY, BUTTON_SIZE.width(), BUTTON_SIZE.height());
    
    // Set tab order
    setTabOrder(this, m_textEdit.get());
    setTabOrder(m_textEdit.get(), m_dropButton.get());
}

void QCheckComboBox::updateDisplayText()
{
    QString displayText = calculateDisplayText();
    m_textEdit->setText(displayText);
    
    // Update size hint if changed
    QSize newSize = sizeHint();
    if (newSize != m_cachedSize) {
        m_cachedSize = newSize;
        updateGeometry();
    }
    
    updateLayout();
}

void QCheckComboBox::notifyDataChanged()
{
    updateDisplayText();
    emit itemStatusChanged();
}

// ============================================================================
// Private Implementation - Popup Management
// ============================================================================

void QCheckComboBox::showPopup()
{
    if (m_popup->isVisible()) {
        return;
    }
    
    QRect popupRect = calculatePopupGeometry();
    QRect startRect(popupRect.topLeft(), QSize(popupRect.width(), 0));
    
    m_animation->setStartValue(startRect);
    m_animation->setEndValue(popupRect);

    m_popup->setGeometry(startRect);
    m_popup->show();
    m_popup->raise();
    m_popup->setFocus();
    
    QApplication::instance()->installEventFilter(this);
    m_animation->start(QAbstractAnimation::KeepWhenStopped);
}

void QCheckComboBox::hidePopup()
{
    if (!m_popup->isVisible()) {
        return;
    }
    
    if (m_animation->state() == QAbstractAnimation::Running) {
        m_animation->stop();
    }

    QApplication::instance()->removeEventFilter(this);
    m_popup->hide();
    setFocus();
}

void QCheckComboBox::togglePopup()
{
    m_popup->isVisible() ? hidePopup() : showPopup();
}

QRect QCheckComboBox::calculatePopupGeometry() const
{
    QPoint globalPos = mapToGlobal(QPoint(0, height()));
    QRect screenRect = screen()->availableGeometry();
    
    // Calculate content height
    int itemCount = m_model->rowCount();
    int contentHeight = itemCount * ITEM_HEIGHT + 4; // 4px for borders
    int popupHeight = qBound(MIN_POPUP_HEIGHT, contentHeight, MAX_POPUP_HEIGHT);
    
    // Check if popup should appear above
    if (globalPos.y() + popupHeight > screenRect.bottom()) {
        int availableSpaceAbove = mapToGlobal(QPoint(0, 0)).y() - screenRect.top();
        if (availableSpaceAbove > popupHeight) {
            globalPos.setY(mapToGlobal(QPoint(0, 0)).y() - popupHeight);
        } else {
            popupHeight = qMin(popupHeight, screenRect.bottom() - globalPos.y());
        }
    }
    
    return QRect(globalPos, QSize(width(), popupHeight));
}

// ============================================================================
// Private Implementation - Helper Methods
// ============================================================================

QString QCheckComboBox::calculateDisplayText() const
{
    QStringList selectedTexts = checkedTexts();
    
    if (selectedTexts.isEmpty()) {
        return m_placeholderText;
    }
    
    if (selectedTexts.size() > MAX_DISPLAY_ITEMS) {
        return QString("%1 / %2").arg(selectedTexts.size()).arg(m_model->rowCount());
    }
    
    return selectedTexts.join(m_separator);
}

QSize QCheckComboBox::calculateSize(bool preferredSize) const
{
    QFontMetrics fm(font());
    QString displayText = calculateDisplayText();
    if (displayText.isEmpty()) {
        displayText = m_placeholderText;
    }
    
    int textWidth = fm.horizontalAdvance(displayText);
    int padding = preferredSize ? PADDING_WIDTH * 2 : PADDING_WIDTH;
    int minWidth = textWidth + BUTTON_SIZE.width() + padding;
    int height = qMax(fm.height() + (preferredSize ? 12 : 8), 
                     BUTTON_SIZE.height() + (preferredSize ? 8 : 4));
    
    if (preferredSize) {
        minWidth = qBound(120, minWidth, 400);
    } else {
        minWidth = qMax(minWidth, MIN_WIDTH);
    }
    
    return QSize(minWidth, height);
}

bool QCheckComboBox::isValidIndex(int index) const
{
    return index >= 0 && index < m_model->rowCount();
}

bool QCheckComboBox::isItemChecked(int index) const
{
    if (!isValidIndex(index)) {
        return false;
    }
    
    QModelIndex modelIndex = m_model->index(index, 0);
    return m_model->data(modelIndex, Qt::CheckStateRole).toInt() == Qt::Checked;
}

void QCheckComboBox::setAllChecked(bool checked)
{
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        m_model->setData(index, state, Qt::CheckStateRole);
    }
    notifyDataChanged();
}

QStandardItem* QCheckComboBox::createItem(const QString& text, const QVariant& data) const
{
    auto item = new QStandardItem();
    item->setData(text, Qt::DisplayRole);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    
    if (!data.isNull()) {
        item->setData(data, Qt::UserRole);
    }
    
    return item;
}

// ============================================================================
// Private Implementation - Event Handling Helpers
// ============================================================================

bool QCheckComboBox::shouldHideOnFocusOut(QFocusEvent* event) const
{
    if (event->reason() == Qt::PopupFocusReason || 
        event->reason() == Qt::MouseFocusReason) {
        return false;
    }
    
    return !isWidgetPartOfComboBox(QApplication::focusWidget());
}

bool QCheckComboBox::isWidgetPartOfComboBox(QWidget* widget) const
{
    if (!widget) {
        return false;
    }
    
    // Check if widget is part of this combo box
    if (widget == this || isAncestorOf(widget)) {
        return true;
    }
    
    // Check if widget is part of popup hierarchy
    QWidget* current = widget;
    while (current) {
        if (current == m_popup.get()) {
            return true;
        }
        current = current->parentWidget();
    }
    
    return false;
}
