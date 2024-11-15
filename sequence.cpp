#include "sequence.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>

namespace{
QList<Sequence> loadAvaliableSequences(){
    const static QString kPath = "./configs/sequences.json";

    QFileInfo fileInfo(kPath);
    QDir dir = fileInfo.dir();
    if(!dir.exists() && !dir.mkpath(".")){
        qDebug() << "failed to mkdir: " << dir.path();
    }

    QFile file(kPath);
    if(!file.exists()){

    }
}
}

Sequence::Sequence() {}

