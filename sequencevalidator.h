#ifndef SEQUENCEVALIDATOR_H
#define SEQUENCEVALIDATOR_H

#include <QJsonObject>

class SequenceValidator
{
public:
    SequenceValidator();
    static bool validate(QJsonObject& sequence);
};

#endif // SEQUENCEVALIDATOR_H
