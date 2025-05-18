#include <QMessageBox>
#include <QVector>

#include "exameditdialog.h"
#include "ui_exameditdialog.h"
#include "utils.h"


ExamEditDialog::ExamEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExamInfoDialog)
{
    ui->setupUi(this);

    connect(ui->editXAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setX(value);
    });

    connect(ui->editYAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setY(value);
    });

    connect(ui->editZAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setZ(value);
    });
    

    connect(ui->editXOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setX(value);
    });

    connect(ui->editYOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setY(value);
    });

    connect(ui->editZOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setZ(value);
    });

    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, [this](int num){
        if(m_slices.count() == num){
            return;
        }

        m_slices.resize(num);
        setSliceComboNumbers(num);
    });

    initScoutWidget();
}

ExamEditDialog::~ExamEditDialog()
{
}

void ExamEditDialog::setData(const Exam& exam)
{
    QJsonObject parameters = exam.request().params();

    ui->editFOV->setValue(parameters[KEY_FOV].toDouble());
    ui->editNoAverages->setValue(parameters[KEY_NO_AVERAGES].toInt());
    ui->editNoSlices->setValue(parameters[KEY_NO_SLICES].toInt());
    ui->editNoSamples->setValue(parameters[KEY_NO_SAMPLES].toInt());
    ui->editNoViews->setValue(parameters[KEY_NO_VIEWS].toInt());
    ui->editObserveFrequency->setValue(parameters[KEY_OBSERVE_FREQUENCY].toDouble());
    ui->editSliceThickness->setValue(parameters[KEY_SLICE_THICKNESS].toDouble());

    if(parameters.contains("slices")){
        ui->checkGroupMode->setChecked(false);
        setSlices(parameters["slices"].toArray());
    }else{
        ui->checkGroupMode->setChecked(true);
        ui->editSliceSeparation->setValue(parameters["sliceSeparation"].toDouble());
    }

    preview();
}

QJsonObject ExamEditDialog::getParameters()
{
    QJsonObject out;

    out.insert(KEY_FOV, ui->editFOV->value());
    out.insert(KEY_NO_AVERAGES, ui->editNoAverages->value());
    out.insert(KEY_NO_SLICES, ui->editNoSlices->value());
    out.insert(KEY_NO_SAMPLES, ui->editNoSamples->value());
    out.insert(KEY_NO_VIEWS, ui->editNoViews->value());
    out.insert(KEY_OBSERVE_FREQUENCY, ui->editObserveFrequency->value());
    out.insert(KEY_SLICE_THICKNESS, ui->editSliceThickness->value());

    if(m_slices.empty()){
        out.insert(KEY_SLICE_SEPARATION, ui->editSliceSeparation->value());
    }else{
        out.insert(KEY_SLICES, jsonSlices());
    }

    return out;
}

/// @warning 这段代码非常死板，只要scout不是T2或者长度不够9就会导致程序崩溃
void ExamEditDialog::setScout(const Exam &exam)
{
    try{
        auto images = exam.images();

        /// @todo 创建类读取params，这样容易出错
        auto params = exam.request().params();
        auto fov = params["fov"].toDouble();
        auto slices = params["slices"].toArray();

        QList<QVector3D> angles;
        QList<QVector3D> offsets;
        for (const auto &slice : slices) {
            auto sliceObj = slice.toObject();
            auto xOffset = sliceObj["xOffset"].toDouble();
            auto yOffset = sliceObj["yOffset"].toDouble();
            auto zOffset = sliceObj["zOffset"].toDouble();
            auto xAngle = sliceObj["xAngle"].toDouble();
            auto yAngle = sliceObj["yAngle"].toDouble();
            auto zAngle = sliceObj["zAngle"].toDouble();

            offsets.push_back(QVector3D(xOffset, yOffset, zOffset));
            angles.push_back(QVector3D(xAngle, yAngle, zAngle));
        }

        ui->scoutWidget->setScoutImages(images[0].sliced(0, 9), fov, angles, offsets);
    }catch(...){
        return;
    }
}

QVector3D ExamEditDialog::offset() const
{
    auto xOffset = ui->editXOffset->value();
    auto yOffset = ui->editYOffset->value();
    auto zOffset = ui->editZOffset->value();
    return QVector3D(xOffset, yOffset, zOffset);
}

void ExamEditDialog::setOffset(QVector3D offset)
{
    if(offset == this->offset()){
        return;
    }

    // 禁用信号防止多次更新，保留一个用于触发信号
    ui->editXOffset->blockSignals(true);
    ui->editYOffset->blockSignals(true);

    ui->editXOffset->setValue(offset.x());
    ui->editYOffset->setValue(offset.y());
    ui->editZOffset->setValue(offset.z());

    // 恢复信号
    ui->editXOffset->blockSignals(false);
    ui->editYOffset->blockSignals(false);
}

QVector3D ExamEditDialog::angle() const
{
    auto xAngle = ui->editXAngle->value();
    auto yAngle = ui->editYAngle->value();
    auto zAngle = ui->editZAngle->value();
    return QVector3D(xAngle, yAngle, zAngle);
}

void ExamEditDialog::setAngle(QVector3D other)
{
    if(other == this->angle()){
        return;
    }

    // 禁用信号防止多次更新，保留一个用于触发信号
    ui->editXAngle->blockSignals(true);
    ui->editYAngle->blockSignals(true);

    ui->editXAngle->setValue(other.x());
    ui->editYAngle->setValue(other.y());
    ui->editZAngle->setValue(other.z());

    // 恢复信号
    ui->editXAngle->blockSignals(false);
    ui->editYAngle->blockSignals(false);

}

void ExamEditDialog::setSlices(QJsonArray slicesArray)
{
    auto sliceNum = slicesArray.count();

    setSliceComboNumbers(sliceNum);

    m_slices.resize(sliceNum);

    for(int i=0;i<sliceNum;i++){
        auto slice = slicesArray[i].toObject();

        QVector3D angles(
            slice[KEY_X_ANGLE].toDouble(),
            slice[KEY_Y_ANGLE].toDouble(),
            slice[KEY_Z_ANGLE].toDouble()
        );

        QVector3D offsets(
            slice[KEY_X_OFFSET].toDouble(),
            slice[KEY_Y_OFFSET].toDouble(),
            slice[KEY_Z_OFFSET].toDouble()
        );

        m_slices[i] = qMakePair(angles, offsets);
    }

    setSliceComboNumbers(slicesArray.count());
}

QJsonArray ExamEditDialog::jsonSlices()
{
    QJsonArray out;
    for(const auto& slice:m_slices){
        QJsonObject item;
        item[KEY_X_ANGLE] = slice.first.x();
        item[KEY_Y_ANGLE] = slice.first.y();
        item[KEY_Z_ANGLE] = slice.first.z();
        item[KEY_X_OFFSET] = slice.second.x();
        item[KEY_Y_OFFSET] = slice.second.y();
        item[KEY_Z_OFFSET] = slice.second.z();
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

    connect(ui->scoutWidget, &ScoutWidget::offsetChanged, this, [this](QVector3D movement){

        this->setOffset(this->offset() + movement);
    });
}

void ExamEditDialog::preview()
{
    LOG_INFO("paint");

    auto fov = ui->editFOV->value();
    auto noSlices = ui->editNoSlices->value();
    auto thickness = ui->editSliceThickness->value();

    if(ui->checkGroupMode->isChecked()){
        // Group Mode
        auto separation = ui->editSliceSeparation->value();

        auto angles = this->angle();
        auto offsets = this->offset();
        ui->scoutWidget->preview(fov, thickness, separation, noSlices, angles, offsets);
    }else{
        QList<QVector3D> angles;
        QList<QVector3D> offsets;

        for(int i=0;i<m_slices.count();i++){
            angles.push_back(m_slices[i].first);
            offsets.push_back(m_slices[i].second);
        }

        ui->scoutWidget->preview(fov, thickness, noSlices, angles, offsets);
    }
}

void ExamEditDialog::on_comboSlice_currentIndexChanged(int index)
{
    const auto& curSlice = m_slices[index];
    
    ui->editXAngle->setValue(curSlice.first.x());
    ui->editYAngle->setValue(curSlice.first.y());
    ui->editZAngle->setValue(curSlice.first.z());
    ui->editXOffset->setValue(curSlice.second.x());
    ui->editYOffset->setValue(curSlice.second.y());
    ui->editZOffset->setValue(curSlice.second.z());
}
