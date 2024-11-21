#include <QMessageBox>

#include "examinfodialog.h"
#include "ui_examinfodialog.h"


ExamInfoDialog::ExamInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExamInfoDialog)
{
    ui->setupUi(this);

    sliceSpinBoxKeyMap = {
        {ui->editXAngle, "xAngle"},
        {ui->editYAngle, "yAngle"},
        {ui->editZAngle, "zAngle"},
        {ui->editXOffset, "xOffset"},
        {ui->editYOffset, "yOffset"},
        {ui->editZOffset, "zOffset"}
    };

    sliceSpinBoxIndexMap = {
        {ui->editXAngle, 0},
        {ui->editYAngle, 1},
        {ui->editZAngle, 2},
        {ui->editXOffset, 3},
        {ui->editYOffset, 4},
        {ui->editZOffset, 5},
    };

    for(const auto&[spinbox, keyIndex]:sliceSpinBoxIndexMap.asKeyValueRange()){
        connect(spinbox, &QDoubleSpinBox::valueChanged, this, [this, spinbox, keyIndex](){
            int index = ui->comboSlice->currentIndex();
            m_slices[index][keyIndex] = spinbox->value();
        });
    }

    paramEditKeyMap = {
        {ui->editFOV, "fov"},
        {ui->editNoAverages, "noAverages"},
        {ui->editNoSlices, "noSlices"},
        {ui->editNoSamples, "noSamples"},
        {ui->editNoViews, "noViews"},
        {ui->editObserveFrequency, "observeFrequency"},
        {ui->editSliceThickness, "sliceThickness"},
    };
}

ExamInfoDialog::~ExamInfoDialog()
{
    delete ui;
}

void ExamInfoDialog::setData(const QJsonObject &exam)
{
    QJsonObject parameters = exam["parameters"].toObject();
    for(const auto&[abstractSpinBox, jsonKey]:paramEditKeyMap.asKeyValueRange()){
        if(QSpinBox* spinBox = qobject_cast<QSpinBox*>(abstractSpinBox)){
            spinBox->setValue(parameters[jsonKey].toInt());
        }else if(QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(abstractSpinBox)){
            doubleSpinBox->setValue(parameters[jsonKey].toDouble());
        }else{
            qDebug() << "set data error";
        }
    }

    if(parameters.contains("slices")){
        ui->checkGroupMode->setChecked(false);
        setSlices(parameters["slices"].toArray());
    }else{
        ui->checkGroupMode->setChecked(true);
        ui->editSliceSeparation->setValue(parameters["sliceSeparation"].toDouble());
    }
}

QJsonObject ExamInfoDialog::getParameters()
{
    QJsonObject out;
    for(const auto&[abstractSpinBox, jsonKey]:paramEditKeyMap.asKeyValueRange()){
        if(QSpinBox* spinBox = qobject_cast<QSpinBox*>(abstractSpinBox)){
            out.insert(jsonKey, spinBox->value());
        }else if(QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(abstractSpinBox)){
            out.insert(jsonKey, doubleSpinBox->value());
        }else{
            qDebug() << "get data error";
        }
    }

    if(m_slices.empty()){
        out.insert("sliceSeparation", ui->editSliceSeparation->value());
    }else{
        out.insert("slices", getSlices());
    }

    return out;
}

void ExamInfoDialog::setSlices(QJsonArray slicesArray)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: set slices at group mode";
    }
    ui->comboSlice->clear();

    // trun json to m_slices(QVector<QVector<double>>)
    for(int i=0;i<slicesArray.count();i++){
        QVector<double> slice(sliceSpinBoxIndexMap.count());
        auto _slice = slicesArray[i].toObject();
        for(const auto& [spinbox, jsonKey]:sliceSpinBoxKeyMap.asKeyValueRange()){
            int index = sliceSpinBoxIndexMap[spinbox];
            slice[index] = _slice[jsonKey].toDouble();
        }
        m_slices.append(slice);
    }
    // set slice choices in combox
    for(int i=0;i<m_slices.count();i++){
        ui->comboSlice->addItem(QString::number(i));
    }
    ui->comboSlice->setCurrentIndex(0);
}

QJsonArray ExamInfoDialog::getSlices()
{
    QJsonArray out;
    for(const auto& slice:m_slices){
        QJsonObject item;
        for(const auto&[spinbox, jsonKey]:sliceSpinBoxKeyMap.asKeyValueRange()){
            int index = sliceSpinBoxIndexMap[spinbox];
            item[jsonKey] = slice[index];
        }
        out.append(item);
    }
    return out;
}

void ExamInfoDialog::on_comboSlice_currentIndexChanged(int index)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: comboBox changed at group mode";
    }

    const auto& curSlice = m_slices[index];
    for(const auto& [spinbox, i]:sliceSpinBoxIndexMap.asKeyValueRange()){
        spinbox->setValue(curSlice[i]);
    }
}

void ExamInfoDialog::on_checkGroupMode_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    if(arg1 == Qt::Checked){
        ui->stackedWidget->setCurrentIndex(1);
        return;
    }

    qDebug() << "unexpected group mode checkbox status: " << arg1;
}
