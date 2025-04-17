#include "qcheckcombobox.h"

#include <QSizePolicy>
#include <QScreen>
#include <QMouseEvent>
#include <QApplication>

QCheckComboBox::QCheckComboBox(QWidget *parent)
    : QWidget(parent),
    m_button(new QPushButton(this)),
    m_text(new QTextEdit(this)),
    m_popup(new QListView(this)),
    m_model(new QStandardItemModel(this))
{
    initButton();
    m_text->setEnabled(false);
    initPopup();

    updateLayout();
}

QCheckComboBox::~QCheckComboBox()
{
    delete m_popup;
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
}

QList<QVariant> QCheckComboBox::values(QCheckComboBox::Filter filter)
{
    auto result = QList<QVariant>();

    for(size_t i=0;i<m_model->rowCount();i++){
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
            if(s.toBool()){
                result.push_back(v);
            }
            break;
        case UNCHECKED:
            if(!s.toBool()){
                result.push_back(v);
            }
        }
    }

    return result;
}

void QCheckComboBox::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateLayout();
}

void QCheckComboBox::mousePressEvent(QMouseEvent *e)
{
    if(!m_popup->isVisible()){
        showPopup();
    }else{
        hidePopup();
    }
    QWidget::mousePressEvent(e);
}

bool QCheckComboBox::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_button && event->type() == QEvent::MouseButtonPress){
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
    emit itemStatusChanged();
}

void QCheckComboBox::updateLayout()
{
    auto margins = contentsMargins();
    int x = width() - m_button->width() - margins.right();
    int y = (height() - m_button->height()) / 2;
    m_button->move(x, y);

    m_text->resize(this->size());
}

void QCheckComboBox::showPopup()
{
    auto animation = new QPropertyAnimation(m_popup, "geometry");
    animation->setDuration(150);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    QPoint globalPos = mapToGlobal(QPoint(0, height()));
    QRect screenRect = screen()->availableGeometry();
    int maxHeight = screenRect.height() - globalPos.y();
    m_popup->setFixedSize(width(), qMin(200, maxHeight));

    QRect startRect(globalPos, QSize(width(), 0));
    QRect endRect(globalPos, QSize(width(), m_popup->height()));
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    m_popup->raise();
    m_popup->show();
}

void QCheckComboBox::hidePopup()
{
    if(!m_popup->isVisible()){
        return;
    }

    m_popup->hide();
}

void QCheckComboBox::initButton()
{
    static QString BUTTON_STYLE = R"(
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
    m_button->setStyleSheet(BUTTON_STYLE);

    m_button->setFixedSize(BUTTON_SIZE);
    m_button->setIcon(QIcon(":/qcheckcombobox.svg"));
    m_button->raise();
    m_button->installEventFilter(this);

}

void QCheckComboBox::initPopup()
{
    m_popup->hide();

    m_popup->setModel(m_model);
    m_popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_popup->installEventFilter(this);
    connect(m_popup, &QListView::clicked, this, &QCheckComboBox::onItemClicked);
}

void QCheckComboBox::passButtonClick(QObject *obj, QEvent *event)
{
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    auto parentPos = m_button->mapToParent(mouseEvent->pos());
    auto globalPos = m_button->mapToGlobal(mouseEvent->pos());
    auto parentEvent = new QMouseEvent(QEvent::MouseButtonPress,
                                       parentPos,
                                       globalPos,
                                       mouseEvent->button(),
                                       mouseEvent->buttons(),
                                       mouseEvent->modifiers()
                                       );
    QApplication::sendEvent(this, parentEvent);
    delete parentEvent;
}

