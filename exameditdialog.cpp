#include <QMessageBox>
#include <QVector>
#include <QMatrix4x4>
#include <QQuaternion>


#include "exameditdialog.h"
#include "ui_exameditdialog.h"
#include "utils.h"


ExamEditDialog::ExamEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExamInfoDialog)
{
    ui->setupUi(this);

    resisterEditerSignals();

    // offset changed
    connect(ui->scoutWidget, &ScoutWidget::offsetChanged, this, [this](QVector3D movement){
        this->shouldRepaint = false;

        this->setOffset(this->offset() + movement);
        this->shouldRepaint = true;
        this->preview();
    });

    // angle changed
    connect(ui->scoutWidget, &ScoutWidget::angleChanged, this, [this](QVector3D angle){

        auto currentIndex = ui->comboSlice->currentIndex();
        auto currentSlice = m_slices[currentIndex];
        auto oldMatrix = ui->scoutWidget->rotateMatrix(currentSlice.first);
        auto newMatrix = oldMatrix * ui->scoutWidget->rotateMatrix(angle);
        // 由矩阵计算角度(x,y,z), 要求旋转顺序为xyz
        QQuaternion quat = QQuaternion::fromRotationMatrix(newMatrix.toGenericMatrix<3,3>());
        auto newAngle = quat.toEulerAngles();

        this->shouldRepaint = false;

        this->setAngle(newAngle);
        this->shouldRepaint = true;
        this->preview();
    });
}

ExamEditDialog::~ExamEditDialog()
{
}

void ExamEditDialog::resisterEditerSignals()
{
    connect(ui->editXAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setX(value);
        preview();
    });

    connect(ui->editYAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setY(value);
        preview();
    });

    connect(ui->editZAngle, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].first.setZ(value);
        preview();
    });

    connect(ui->editXOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setX(value);
        preview();
    });

    connect(ui->editYOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setY(value);
        preview();
    });

    connect(ui->editZOffset, &QDoubleSpinBox::valueChanged, this, [this](double value){
        int index = ui->comboSlice->currentIndex();
        m_slices[index].second.setZ(value);
        preview();
    });

    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, [this](int num){
        if(m_slices.count() == num){
            return;
        }

        m_slices.resize(num);
        setSliceComboNumbers(num);
        preview();
    });

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

    if(parameters.contains(KEY_SLICES)){
        ui->checkGroupMode->setChecked(false);
        setSlices(parameters[KEY_SLICES].toArray());
    }else{
        ui->checkGroupMode->setChecked(true);
        ui->editSliceSeparation->setValue(parameters[KEY_SLICE_SEPARATION].toDouble());
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
        auto fov = params[KEY_FOV].toDouble();
        auto slices = params[KEY_SLICES].toArray();

        QList<QVector3D> angles;
        QList<QVector3D> offsets;
        for (const auto &slice : slices) {
            auto sliceObj = slice.toObject();
            auto xOffset = sliceObj[KEY_X_OFFSET].toDouble();
            auto yOffset = sliceObj[KEY_Y_OFFSET].toDouble();
            auto zOffset = sliceObj[KEY_Z_OFFSET].toDouble();
            auto xAngle = sliceObj[KEY_X_ANGLE].toDouble();
            auto yAngle = sliceObj[KEY_Y_ANGLE].toDouble();
            auto zAngle = sliceObj[KEY_Z_ANGLE].toDouble();

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

    ui->editXOffset->setValue(offset.x());
    ui->editYOffset->setValue(offset.y());
    ui->editZOffset->setValue(offset.z());
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

    ui->editXAngle->setValue(other.x());
    ui->editYAngle->setValue(other.y());
    ui->editZAngle->setValue(other.z());
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

void ExamEditDialog::preview()
{
    if(!shouldRepaint){
        return;
    }

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
        auto currentIndex = ui->comboSlice->currentIndex();
        ui->scoutWidget->preview(fov, thickness, {m_slices[currentIndex]});
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
