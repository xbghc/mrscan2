#ifndef ABSTRACTPREFERENCESTAB_H
#define ABSTRACTPREFERENCESTAB_H

#include <QWidget>
#include <QJsonObject>

class AbstractPreferencesTab : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractPreferencesTab(QWidget *parent = nullptr);
    virtual void set(QJsonObject pref)=0;
    virtual QJsonObject get()=0;

signals:
};

#endif // ABSTRACTPREFERENCESTAB_H
