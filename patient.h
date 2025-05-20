#ifndef PATIENT_H
#define PATIENT_H

#include <QDate>
#include <QJsonObject>
#include <QList>

class IPatient {
public:
    enum class Gender { Male = 0, Female };

    virtual ~IPatient() = default;
    virtual IPatient *clone() const = 0;

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual Gender gender() const = 0;
    virtual QDate birthday() const = 0;

    virtual void setId(const QString &other) = 0;
    virtual void setName(const QString &other) = 0;
    virtual void setGender(Gender other) = 0;
    virtual void setBirthday(QDate other) = 0;
    virtual void setBirthday(int year, int month, int day) = 0;

    virtual QByteArray bytes() const = 0;

protected:
    IPatient() = default;
};

/**
 * @brief QJsonObject版本的临时使用的Patient
 * @note 病人数据将来会从外部获取，并重新实现IPatient
 */
class JsonPatient : public IPatient {
public:
    struct Keys {
        const static QString Id;
        const static QString Name;
        const static QString Gender;
        const static QString Birthday;
    };
    static constexpr auto kDateFormat = "yyyy-MM-dd";

    JsonPatient() = default;
    JsonPatient(QJsonObject m_data);
    IPatient *clone() const override;

    QString id() const override;
    QString name() const override;
    Gender gender() const override;
    QDate birthday() const override;

    void setId(const QString &other) override;
    void setName(const QString &other) override;
    void setGender(Gender other) override;
    void setBirthday(int year, int month, int day) override;
    void setBirthday(QDate other) override;

    QByteArray bytes() const override;
private:
    QJsonObject m_data;
};

#endif // PATIENT_H
