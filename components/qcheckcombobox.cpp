#include "qcheckcombobox.h"

#include <QSizePolicy>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QFontMetrics>

const QString QCheckComboBox::BUTTON_STYLE = R"(
QPushButton {
    border: none;
    background: #ffffff;
}
QPushButton:hover {
    background: #eeeeee;
}
QPushButton:pressed {
    background: #dddddd;
}
)";

QCheckComboBox::QCheckComboBox(QWidget *parent)
    : QWidget(parent),
    m_text(std::make_unique<QTextEdit>(this)),
    m_button(std::make_unique<QPushButton>(this)),
    m_popup(std::make_unique<QListView>(this)),
    m_model(std::make_unique<QStandardItemModel>(this)),
    m_animation(std::make_unique<QPropertyAnimation>(this)),
    m_separator(", ")
{
    initButton();
    initTextEdit();
    initPopup();
    
    // init animation object
    m_animation->setTargetObject(m_popup.get());
    m_animation->setPropertyName("geometry");
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);

    updateLayout();
    setFocusPolicy(Qt::StrongFocus);
}

QCheckComboBox::~QCheckComboBox()
{
}

void QCheckComboBox::addItem(const QString &text, const QVariant &data)
{
    auto item = new QStandardItem();
    item->setData(text, Qt::DisplayRole);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if(!data.isNull()){
        item->setData(data, Qt::UserRole);
    }
    m_model->appendRow(item);
    updateDisplayText();
}

void QCheckComboBox::setItems(const QStringList &items)
{
    m_model->clear();
    for (const QString &text : items) {
        addItem(text);
    }
    updateDisplayText();
    emit itemStatusChanged();
}

QList<QVariant> QCheckComboBox::values(QCheckComboBox::Filter filter)
{
    auto result = QList<QVariant>();

    for(int i=0; i<m_model->rowCount(); i++){
        auto index = m_model->index(i, 0);
        auto v = m_model->data(index, Qt::UserRole);
        if(v.isNull()){
            v = m_model->data(index, Qt::DisplayRole);
        }
        auto s = m_model->data(index, Qt::CheckStateRole);

        switch(filter){
        case ALL:
            result.append(v);
            break;
        case CHECKED:
            if(s.toInt() == Qt::Checked){
                result.push_back(v);
            }
            break;
        case UNCHECKED:
            if(s.toInt() != Qt::Checked){
                result.push_back(v);
            }
            break;
        }
    }

    return result;
}

int QCheckComboBox::itemCount() const
{
    return m_model->rowCount();
}

void QCheckComboBox::setChecked(int index, bool checked)
{
    if (index < 0 || index >= m_model->rowCount()) {
        return;
    }
    
    QModelIndex modelIndex = m_model->index(index, 0);
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    m_model->setData(modelIndex, state, Qt::CheckStateRole);
    
    // Update display text
    updateDisplayText();
    
    // Trigger status change signal
    emit itemStatusChanged();
}

void QCheckComboBox::removeAllItems()
{
    m_model->clear();
    updateDisplayText();
    emit itemStatusChanged();
}

bool QCheckComboBox::isChecked(int index) const
{
    if (index < 0 || index >= m_model->rowCount()) {
        return false;
    }
    
    QModelIndex modelIndex = m_model->index(index, 0);
    QVariant checkState = m_model->data(modelIndex, Qt::CheckStateRole);
    return checkState.toInt() == Qt::Checked;
}

void QCheckComboBox::resizeEvent(QResizeEvent *e)
{
    updateLayout();
    
    QSize popupSize = m_popup->size();
    popupSize.setWidth(size().width());
    m_popup->resize(popupSize);

    QPropertyAnimation::State state = m_animation->state();
    if (state != QPropertyAnimation::Stopped) {
        QRect startRect = m_animation->startValue().toRect();
        startRect.setWidth(size().width());
        m_animation->setStartValue(startRect);

        QRect endRect = m_animation->endValue().toRect();
        endRect.setWidth(size().width());
        m_animation->setEndValue(endRect);
    }

    QWidget::resizeEvent(e);
}

void QCheckComboBox::mousePressEvent(QMouseEvent *e)
{
    if (!m_popup->isVisible()) {
        showPopup();
    } else {
        hidePopup();
    }
    e->accept();
}

bool QCheckComboBox::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_button.get() && event->type() == QEvent::MouseButtonPress){
        passButtonClick(obj, event);
        return true;
    }
    if(event->type()==QEvent::MouseButtonPress && m_popup->isVisible()){
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if(!m_popup->geometry().contains(mouseEvent->globalPosition().toPoint())){
            hidePopup();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void QCheckComboBox::onItemClicked(const QModelIndex& index)
{
    auto v = m_model->data(index, Qt::CheckStateRole);
    auto checkStatus = qvariant_cast<Qt::CheckState>(v);
    if(checkStatus == Qt::Unchecked){
        m_model->setData(index, Qt::Checked, Qt::CheckStateRole);
    }else if(checkStatus == Qt::Checked){
        m_model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }else{
        qDebug() << "unsupport check status: " << checkStatus;
    }
    updateDisplayText();
    emit itemStatusChanged();
}

void QCheckComboBox::updateLayout()
{
    auto margins = contentsMargins();
    
    // Calculate text box size and position, ensure vertical centering
    int textWidth = width() - margins.right() - margins.left();
    int textHeight = m_text->document()->size().toSize().height() + 4; // Consider actual content height
    int maxTextHeight = height() - margins.top() - margins.bottom();
    textHeight = qMin(textHeight, maxTextHeight);
    
    // Vertical centering calculation
    int textY = margins.top() + (maxTextHeight - textHeight) / 2;
    
    m_text->resize(textWidth, textHeight);
    m_text->move(margins.left(), textY);
    
    // Set button position, aligned with the right side of the text box and vertically centered
    m_button->setFixedHeight(textHeight-4);
    int buttonX = width() - m_button->width() - margins.right() - 4;
    int buttonY = (height() - m_button->height()) / 2;
    m_button->move(buttonX, buttonY);
    
    // Set appropriate focus order
    setTabOrder(this, m_text.get());
    setTabOrder(m_text.get(), m_button.get());
}

void QCheckComboBox::showPopup()
{
    // First automatically adjust item height based on content
    adjustItemsHeight();
    
    QPoint globalPos = mapToGlobal(QPoint(0, height()));
    QRect screenRect = screen()->availableGeometry();
    
    QFontMetrics fm(font());
    int itemHeight = fm.height() + 8; // Keep consistent with adjustItemsHeight
    int itemCount = m_model->rowCount();
    int contentHeight = itemCount * itemHeight;
    
    // Add border space
    if (itemCount > 0) {
        contentHeight += 2;
    }
    
    // Set reasonable minimum and maximum height
    int minHeight = 30;
    int maxHeight = qMin(screenRect.height() - globalPos.y(), 200);

    int popupHeight = qMin(qMax(contentHeight, minHeight), maxHeight);

    QRect startRect(globalPos, QSize(width(), 0));
    QRect endRect(globalPos, QSize(width(), popupHeight));
    m_animation->setStartValue(startRect);
    m_animation->setEndValue(endRect);

    // Use KeepWhenStopped to avoid deleting member variables
    m_popup->setGeometry(startRect);
    m_popup->show();
    m_popup->raise();
    m_animation->start(QAbstractAnimation::KeepWhenStopped);
}

void QCheckComboBox::hidePopup()
{
    if(!m_popup->isVisible()){
        return;
    }
    
    // Stop current animation (if any)
    if (m_animation && m_animation->state() == QAbstractAnimation::Running) {
        m_animation->stop();
    }

    m_popup->hide();
}

void QCheckComboBox::initButton()
{
    m_button->setStyleSheet(BUTTON_STYLE);

    m_button->setFixedSize(BUTTON_SIZE);
    m_button->setIcon(QIcon(":/qcheckcombobox.svg"));
    m_button->setIconSize(QSize(16, 16));
    m_button->raise();
    m_button->installEventFilter(this);
}

void QCheckComboBox::initPopup()
{
    m_popup->hide();

    m_popup->setModel(m_model.get());
    m_popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_popup->setUniformItemSizes(true);
    
    m_popup->installEventFilter(this);
    connect(m_popup.get(), &QListView::clicked, this, &QCheckComboBox::onItemClicked);
}

void QCheckComboBox::passButtonClick(QObject *obj, QEvent *event)
{
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    auto parentPos = m_button->mapToParent(mouseEvent->pos());
    auto globalPos = m_button->mapToGlobal(mouseEvent->pos());
    QMouseEvent parentEvent(QEvent::MouseButtonPress,
                           parentPos,
                           globalPos,
                           mouseEvent->button(),
                           mouseEvent->buttons(),
                           mouseEvent->modifiers()
                           );
    QApplication::sendEvent(this, &parentEvent);
}

void QCheckComboBox::updateDisplayText()
{
    QStringList selectedTexts;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        if (m_model->data(index, Qt::CheckStateRole).toInt() == Qt::Checked) {
            selectedTexts << m_model->data(index, Qt::DisplayRole).toString();
        }
    }
    
    m_text->setText(selectedTexts.join(m_separator));
    updateLayout(); // Update layout to ensure text is centered
}

void QCheckComboBox::setButtonIcon(const QIcon &icon)
{
    m_button->setIcon(icon);
}

void QCheckComboBox::selectAll()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        m_model->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
    updateDisplayText();
    emit itemStatusChanged();
}

void QCheckComboBox::deselectAll()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        m_model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }
    updateDisplayText();
    emit itemStatusChanged();
}

void QCheckComboBox::setSeparator(const QString &separator)
{
    m_separator = separator;
    updateDisplayText();
}

void QCheckComboBox::initTextEdit()
{
    static const QString TEXT_STYLE = R"(
    QTextEdit {
        border: 1px solid #c0c0c0;
        border-radius: 3px;
        padding: 2px 4px;
        background: white;
        margin: 0px;
    }
    )";
    
    m_text->setStyleSheet(TEXT_STYLE);
    m_text->setEnabled(false);
    m_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_text->setLineWrapMode(QTextEdit::NoWrap);
    
    // Adjust text box content alignment
    QTextOption option = m_text->document()->defaultTextOption();
    option.setAlignment(Qt::AlignVCenter);
    m_text->document()->setDefaultTextOption(option);
    
    // Remove internal padding
    m_text->document()->setDocumentMargin(0);
}

void QCheckComboBox::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!m_popup->isVisible()) {
            showPopup();
        } else {
            hidePopup();
        }
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
    }
}

void QCheckComboBox::adjustItemsHeight()
{
    // Directly use current font to calculate item height
    QFontMetrics fm(font());
    int textHeight = fm.height();
    
    // Add margin to get final height
    int itemHeight = textHeight + 8;
    
    // Apply uniform item height
    QString styleSheet = QString("QListView::item { height: %1px; }").arg(itemHeight);
    m_popup->setStyleSheet(styleSheet);
}

