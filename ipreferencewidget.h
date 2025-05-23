#ifndef IPREFERENCEWIDGET_H
#define IPREFERENCEWIDGET_H

#include <QWidget>

class IPreferenceWidget : public QWidget
{
    Q_OBJECT
protected:
    explicit IPreferenceWidget(QWidget *parent = nullptr);

public:
    virtual ~IPreferenceWidget() = default;
    virtual void save() = 0;
    virtual void load() = 0;
signals:
};

#endif // IPREFERENCEWIDGET_H
