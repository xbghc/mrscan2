#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include "sequencevalidator.h"


namespace{
bool sequenceExists(QJsonObject& sequence){
    QFile file("./configs/sequences.json");
    if(!file.exists() || !file.open(QIODevice::ReadOnly)){
        qDebug() << "validator can't open file: ./configs/sequences.json";
        return false;
    }

    QJsonArray avaliableSequences = QJsonDocument::fromJson(file.readAll()).array();
    for(auto s:avaliableSequences){
        if(s.toObject()["name"] == sequence["sequence"]){
            return true;
        }
    }
    return false;
}

bool validateTune(QJsonObject& parameters){

    return true;
}

bool validateRfopt(QJsonObject& parameters){
    const static QStringList kKeys = {
        "observeFrequency",
    };

    for(auto& k:kKeys){
        if(!parameters.contains(k)){
            qDebug() << "T1 sequence miss parameter: " << k;
            return false;
        }
    }

    return true;
}

bool validateShim(QJsonObject& parameters){
    const static QStringList kKeys = {
        "observeFrequency",
    };

    for(auto& k:kKeys){
        if(!parameters.contains(k)){
            qDebug() << "T1 sequence miss parameter: " << k;
            return false;
        }
    }

    return true;
}


bool validateT1(QJsonObject& parameters){
    const static QStringList kKeys = {
        "observeFrequency",
        "noSamples",
        "noViews",
        "noViews2",
        "noAverages",
        "sliceThickness",
        "sliceSeparation",
        "fov",
        "noSlices",
        "xAngle",
        "yAngle",
        "zAngle",
        "xOffset",
        "yOffset",
        "zOffset"
    };

    for(auto& k:kKeys){
        if(!parameters.contains(k)){
            qDebug() << "T1 sequence miss parameter: " << k;
            return false;
        }
    }

    return true;
}

bool validateT2(QJsonObject& parameters){
    const static QStringList kKeys = {
        "observeFrequency",
        "noSamples",
        "noViews",
        "viewsPerSegment",
        "noAverages",
        "sliceThickness",
        "fov",
        "noSlices",
        "slices"
    };

    for(auto& k:kKeys){
        if(!parameters.contains(k)){
            qDebug() << "T2 sequence miss parameter: " << k;
            return false;
        }
    }

    return true;
}

}

SequenceValidator::SequenceValidator() {}

bool SequenceValidator::validate(QJsonObject &sequence)
{
    if(!sequenceExists(sequence)){
        qDebug() << "sequence doesn't exist";
        return false;
    }

    QString seqName = sequence["sequence"].toString();
    QJsonObject parameters = sequence["parameters"].toObject();

    if(seqName == "t1"){
        return validateT1(parameters);
    }

    if(seqName == "t2"){
        return validateT2(parameters);
    }

    if(seqName == "tune"){
        return validateTune(parameters);
    }

    if(seqName == "rfopt"){
        return validateRfopt(parameters);
    }

    if(seqName == "shim"){
        return validateShim(parameters);
    }

    qDebug() << "sequence exists but has no validator";
    return false;
}
