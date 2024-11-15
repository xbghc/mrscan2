#include <QMessageBox>

#include "examinfodialog.h"
#include "ui_examinfodialog.h"

namespace{
void setNumber(QLabel* label, QJsonObject& exam, QString key){
    label->setText(QString::number(exam[key].toInt()));
}
}

ExamInfoDialog::ExamInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExamInfoDialog)
{
    ui->setupUi(this);

    disconnect(ui->buttonBox, &QDialogButtonBox::accepted,
               this, &QDialog::accept);
}

ExamInfoDialog::~ExamInfoDialog()
{
    delete ui;
}

void ExamInfoDialog::setData(QJsonObject &exam)
{
    QJsonObject parameters = exam["parameters"].toObject();
    ui->editFOV->setValue(parameters["fov"].toDouble());
    ui->editNoAverages->setValue(parameters["noAverages"].toInt());
    ui->editNoSlices->setValue(parameters["noSlices"].toInt());
    ui->editNoSamples->setValue(parameters["noSamples"].toInt());
    ui->editNoViews->setValue(parameters["noViews"].toInt());
    ui->editObserveFrequency->setValue(parameters["observeFrequency"].toDouble());
    ui->editSliceThickness->setValue(parameters["sliceThickness"].toDouble());

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

    out.insert("observeFrequency", ui->editObserveFrequency->value());
    out.insert("sliceThickness", ui->editSliceThickness->value());
    out.insert("fov", ui->editFOV->value());
    out.insert("noSamples", ui->editNoSamples->value());
    out.insert("noAverages", ui->editNoAverages->value());
    out.insert("noSlices", ui->editNoSlices->value());

    if(slices.empty()){
        out.insert("sliceSeparation", ui->editSliceSeparation->value());
    }else{
        out.insert("slices", slices);
    }

    return out;
}

void ExamInfoDialog::setSlices(QJsonArray _slices)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: set slices at group mode";
    }

    slices = _slices;
    ui->comboSlice->clear();
    for(int i=0;i<slices.count();i++){
        ui->comboSlice->addItem(QString::number(i));
    }
    ui->comboSlice->setCurrentIndex(0);
}

bool ExamInfoDialog::validate()
{
    return true;
}

void ExamInfoDialog::on_comboSlice_currentIndexChanged(int index)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: comboBox changed at group mode";
    }

    QJsonObject curSlice = slices[index].toObject();
    ui->editXOffset->setValue(curSlice["xOffset"].toDouble());
    ui->editXAngle->setValue(curSlice["xAngle"].toDouble());
    ui->editYOffset->setValue(curSlice["yOffset"].toDouble());
    ui->editYAngle->setValue(curSlice["yAngle"].toDouble());
    ui->editZOffset->setValue(curSlice["zOffset"].toDouble());
    ui->editZAngle->setValue(curSlice["zAngle"].toDouble());
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


void ExamInfoDialog::on_buttonBox_accepted()
{
    if(!validate()){
        QMessageBox::warning(this, "invalid", "invalid parameters!");
    }

    accept();
}


void ExamInfoDialog::on_editXAngle_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["xAngle"] = arg1;
    slices.replace(curIndex, _slice);
}


void ExamInfoDialog::on_editYAngle_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["yAngle"] = arg1;
    slices.replace(curIndex, _slice);
}


void ExamInfoDialog::on_editZAngle_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["zAngle"] = arg1;
    slices.replace(curIndex, _slice);
}


void ExamInfoDialog::on_editXOffset_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["xOffset"] = arg1;
    slices.replace(curIndex, _slice);
}


void ExamInfoDialog::on_editYOffset_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["yOffset"] = arg1;
    slices.replace(curIndex, _slice);
}


void ExamInfoDialog::on_editZOffset_valueChanged(double arg1)
{
    int curIndex = ui->comboSlice->currentIndex();
    QJsonObject _slice = slices[curIndex].toObject();
    _slice["zOffset"] = arg1;
    slices.replace(curIndex, _slice);
}

