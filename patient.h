#ifndef PATIENT_H
#define PATIENT_H

#include <QDate>
#include <QJsonObject>
#include <QList>

class IPatient
{
public:
    enum class Gender{
        Male=0,
        Female
    };

    virtual ~IPatient() = default;
    virtual IPatient* clone()const = 0;

    virtual QString id() = 0;
    virtual QString name() = 0;
    virtual Gender gender() = 0;
    virtual QDate birthday() = 0;

    virtual void setId(QString other) = 0;
    virtual void setName(QString other) = 0;
    virtual void setGender(Gender other) = 0;
    virtual void setBirthday(int year, int month, int day) = 0;

    const static QString kDirPath;
protected:
    IPatient() = default;
};


/**
 * @brief QJsonObject版本的临时使用的Patient
 * @details 病人数据将来会从外部获取，并重新实现IPatient
 * id底层是int
 * @todo JsonPatient的内部是用QJsonObject存储的，
 * 一不注意就会被复制，导致修改无法同步，应该设法规避
 */
class JsonPatient: public IPatient{
public:
    struct Keys{
        const static QString Id;
        const static QString Name;
        const static QString Gender;
        const static QString Birthday;
    };
    static constexpr auto kDateFormat="yyyy-MM-dd";

    JsonPatient();
    explicit JsonPatient(QJsonObject data);
    explicit JsonPatient(QJsonObject&& data);
    IPatient* clone() const override;

    static int nextId();
    /// @todo 下面这些函数都会被清除
    static QVector<JsonPatient> loadPatients();
    static void savePatients(QVector<JsonPatient> patients);

    /**
     * @brief 返回id，若返回负数，表示没有id
     */
    QString id() override;
    QString name() override;
    Gender gender() override;
    QDate birthday() override;

    QJsonObject json();

    void setId(int other);
    void setId(QString other) override;
    void setName(QString other) override;
    void setGender(Gender other) override;
    void setBirthday(int year, int month, int day) override;
    void setBirthday(QDate other);
private:
    QJsonObject m_data;
};

#endif // PATIENT_H
