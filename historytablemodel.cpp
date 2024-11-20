#include "historytablemodel.h"

#include <QDir>
#include <QJsonDocument>

void HistoryTableModel::saveExam(const unsigned char *header, const unsigned char *data, QJsonObject examObj, int patientId)
{
    if(header[0] != 'N' || header[1] != 'M' || header[2] != 'R'){
        qDebug() << "wrong response header";
        return;
    }

    if(header[3]!=1){
        qDebug() << "unsupport response version";
        return;
    }

    int id;
    memcpy(&id, header+4, 4);

    int dataType;
    memcpy(&dataType, header+8, 4);
    if(dataType != 1){
        qDebug() << "unsupported response data type";
        return;
    }

    int dataSize;
    memcpy(&dataSize, header+12, 4);

    int count;
    memcpy(&count, data, 4);

    int width;
    memcpy(&width, data+4, 4);

    int height;
    memcpy(&height, data+8, 4);

    examObj["response"] = QJsonObject{{"dataType", dataType},
                                      {"dataSize", dataSize},
                                      {"count", count},
                                      {"width", width},
                                      {"height", height}};

    // create directory
    QDir dir(getExamDirPath(patientId, id));
    if(!dir.mkpath(".")){
        qDebug() << "failed to mkdir: " << dir.path();
        return;
    }

    // save info
    QFile infoFile(getRequestFilePath(patientId, id));
    if(!infoFile.open(QIODeviceBase::WriteOnly)){
        qDebug() << "failed to open file: " << infoFile.fileName();
        return;
    }
    QJsonDocument infoDoc(examObj);
    infoFile.write(infoDoc.toJson(QJsonDocument::Indented));

    // save dir
    QFile dataFile(getResponseFilePath(patientId, id));
    if(!dataFile.open(QIODevice::WriteOnly)){
        qDebug() << "failed to open file: " << dataFile.fileName();
    }
    dataFile.write(reinterpret_cast<const char*>(data));
}

HistoryTableModel::HistoryTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers << "scan id" << "patient id" << "patient name" << "scan datetime";
}

QVariant HistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section >= 0 && section < m_headers.size()) {
        return m_headers[section];
    }

    return QVariant();
}

int HistoryTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_examHistoryList.count();
}

int HistoryTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_headers.count();
}

QVariant HistoryTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}

QString HistoryTableModel::getExamDirPath(int patientId, int examId)
{
    return QString("./patients/%1/%2").arg(patientId).arg(examId);
}

QJsonObject HistoryTableModel::loadExamInfo(int patientId, int examId)
{
    // get info
    QFile infoFile(getRequestFilePath(patientId, examId));
    if(!infoFile.exists() || infoFile.open(QIODevice::ReadOnly)){
        qDebug() << "<history model>failed to open file: " << infoFile.fileName();
        return QJsonObject();
    }
    QJsonDocument doc = QJsonDocument::fromJson(infoFile.readAll());
    return doc.object();
}

QVariant HistoryTableModel::loadExamData(int patientId, int examId, QJsonObject examObj)
{
    QJsonObject dataInfo = examObj["response"].toObject();
    int dateType = dataInfo["dataType"].toInt();


    return QVariant();
}

QString HistoryTableModel::getResponseFilePath(int patientId, int examId) {
    return QString("./patients/%1/%2/data.dat").arg(patientId).arg(examId);
}

QString HistoryTableModel::getRequestFilePath(int patientId, int examId)
{
    return QString("./patients/%1/%2/info.json").arg(patientId).arg(examId);
}
