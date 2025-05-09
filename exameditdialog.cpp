#include <QMessageBox>

#include "exameditdialog.h"
#include "ui_exameditdialog.h"


ExamEditDialog::ExamEditDialog(QWidget *parent)
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

    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, [this](int num){
        if(m_slices.count() == num){
            return;
        }

        m_slices.resize(num);
        setSliceComboNumbers(num);
    });

    initScoutWidget();

    // TODO 判断是否已经进行了scout扫描，如果有，设置图片
}

ExamEditDialog::~ExamEditDialog()
{
}

void ExamEditDialog::setData(const QJsonObject &exam)
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

QJsonObject ExamEditDialog::getParameters()
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

void ExamEditDialog::setScoutImages(QList<QImage> images, double fov, QList<QVector3D> angles, QList<QVector3D> offsets)
{
    ui->scoutWidget->setScoutImages(images, fov, angles, offsets);

    preview();
}

void ExamEditDialog::setSlices(QJsonArray slicesArray)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: set slices at group mode";
    }

    size_t sliceCount = slicesArray.count();
    if(m_slices.count() != sliceCount){
        m_slices.resize(slicesArray.count());
    }

    for(int sliceIndex=0;sliceIndex<slicesArray.count();sliceIndex++){
        auto slice = slicesArray[sliceIndex].toObject();

        for(const auto& [spinbox, jsonKey]:sliceSpinBoxKeyMap.asKeyValueRange()){
            int index = sliceSpinBoxIndexMap[spinbox];
            m_slices[sliceIndex][index] = slice[jsonKey].toDouble();
        }
    }

    setSliceComboNumbers(slicesArray.count());
}

QJsonArray ExamEditDialog::getSlices()
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

void ExamEditDialog::setSliceComboNumbers(int n)
{
    if (n < 0) return;


    const QSignalBlocker blocker(ui->comboSlice);
    ui->comboSlice->clear();

    QStringList numbers;
    numbers.reserve(n);

    for(int i = 0; i < n; i++) {
        numbers << QString::number(i);
    }

    ui->comboSlice->addItems(numbers);

    if (n > 0) {
        ui->comboSlice->setCurrentIndex(0);
    }
}

void ExamEditDialog::initScoutWidget()
{
    connect(ui->editFOV, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editSliceThickness, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editSliceSeparation, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editXAngle, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editYAngle, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editZAngle, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editXOffset, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editYOffset, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
    connect(ui->editZOffset, &QDoubleSpinBox::valueChanged, this, &ExamEditDialog::preview);
}

void ExamEditDialog::preview()
{
    auto fov = ui->editFOV->value();
    auto noSlices = ui->editNoSlices->value();
    auto thickness = ui->editSliceThickness->value();

    if(ui->checkGroupMode->isChecked()){
        // Group Mode
        auto separation = ui->editSliceSeparation->value();

        auto angles = QVector3D(ui->editXAngle->value(), ui->editYAngle->value(), ui->editZAngle->value());
        auto offsets = QVector3D(ui->editXOffset->value(), ui->editYOffset->value(), ui->editZOffset->value());
        ui->scoutWidget->preview(fov, thickness, separation, noSlices, angles, offsets);
    }else{
        QList<QVector3D> angles;
        QList<QVector3D> offsets;

        for(int i=0;i<m_slices.count();i++){
            angles.push_back(QVector3D(m_slices[i][0], m_slices[i][1], m_slices[i][2]));
            offsets.push_back(QVector3D(m_slices[i][3], m_slices[i][4], m_slices[i][5]));
        }

        ui->scoutWidget->preview(fov, thickness, noSlices, angles, offsets);
    }
}

void ExamEditDialog::on_comboSlice_currentIndexChanged(int index)
{
    if(ui->checkGroupMode->isChecked()){
        qDebug() << "unexpected status: comboBox changed at group mode";
    }

    const auto& curSlice = m_slices[index];
    for(const auto& [spinbox, i]:sliceSpinBoxIndexMap.asKeyValueRange()){
        spinbox->setValue(curSlice[i]);
    }
}

void ExamEditDialog::on_checkGroupMode_stateChanged(int arg1)
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
