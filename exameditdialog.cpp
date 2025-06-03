#include <QMatrix4x4>
#include <QMessageBox>
#include <QQuaternion>
#include <QVector>

#include "exameditdialog.h"
#include "ui_exameditdialog.h"
#include "utils.h"

ExamEditDialog::ExamEditDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExamInfoDialog) {
    ui->setupUi(this);

    setupConnections();
}

ExamEditDialog::~ExamEditDialog() {}

void ExamEditDialog::setupConnections() {
    connect(ui->editXAngle, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setAngle(this->angle());
    });

    connect(ui->editYAngle, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setAngle(this->angle());
    });

    connect(ui->editZAngle, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setAngle(this->angle());
    });

    connect(ui->editXOffset, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setOffset(this->offset());
    });

    connect(ui->editYOffset, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setOffset(this->offset());
    });

    connect(ui->editZOffset, &QDoubleSpinBox::valueChanged, this,
            [this](double value) {
                int index = ui->comboSlice->currentIndex();
        m_slices[index]->setOffset(this->offset());
    });

    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, [this](int num) {
        if (m_slices.count() == num) {
            return;
        }

        m_slices.resize(num);
        setSliceComboNumbers(num);
    });
}

void ExamEditDialog::setData(const Exam &exam) {
    QJsonObject parameters = exam.request().params();

    ui->editFOV->setValue(parameters[KEY_FOV].toDouble());
    ui->editNoAverages->setValue(parameters[KEY_NO_AVERAGES].toInt());
    ui->editNoSlices->setValue(parameters[KEY_NO_SLICES].toInt());
    ui->editNoSamples->setValue(parameters[KEY_NO_SAMPLES].toInt());
    ui->editNoViews->setValue(parameters[KEY_NO_VIEWS].toInt());
    ui->editObserveFrequency->setValue(
        parameters[KEY_OBSERVE_FREQUENCY].toDouble());
    ui->editSliceThickness->setValue(parameters[KEY_SLICE_THICKNESS].toDouble());

    if (parameters.contains(KEY_SLICES)) {
        ui->checkGroupMode->setChecked(false);
        setSlices(parameters[KEY_SLICES].toArray());
    } else {
        ui->checkGroupMode->setChecked(true);
        ui->editSliceSeparation->setValue(
            parameters[KEY_SLICE_SEPARATION].toDouble());
    }
}

QJsonObject ExamEditDialog::getParameters() {
    QJsonObject out;

    out.insert(KEY_FOV, ui->editFOV->value());
    out.insert(KEY_NO_AVERAGES, ui->editNoAverages->value());
    out.insert(KEY_NO_SLICES, ui->editNoSlices->value());
    out.insert(KEY_NO_SAMPLES, ui->editNoSamples->value());
    out.insert(KEY_NO_VIEWS, ui->editNoViews->value());
    out.insert(KEY_OBSERVE_FREQUENCY, ui->editObserveFrequency->value());
    out.insert(KEY_SLICE_THICKNESS, ui->editSliceThickness->value());

    if (m_slices.empty()) {
        out.insert(KEY_SLICE_SEPARATION, ui->editSliceSeparation->value());
    } else {
        out.insert(KEY_SLICES, jsonSlices());
    }

    return out;
}

void ExamEditDialog::setScout(const Exam &exam) {
    try {
        auto images = exam.images();

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
        ui->scoutWidget->setScouts(images[0], fov, angles, offsets);
    } catch (...) {
        LOG_ERROR("setScout failed");
        return;
    }
}

QVector3D ExamEditDialog::offset() const {
    auto xOffset = ui->editXOffset->value();
    auto yOffset = ui->editYOffset->value();
    auto zOffset = ui->editZOffset->value();
    return QVector3D(xOffset, yOffset, zOffset);
}

void ExamEditDialog::setOffset(QVector3D offset) {
    if (offset == this->offset()) {
        return;
    }

    ui->editXOffset->setValue(offset.x());
    ui->editYOffset->setValue(offset.y());
    ui->editZOffset->setValue(offset.z());
}

QVector3D ExamEditDialog::angle() const {
    auto xAngle = ui->editXAngle->value();
    auto yAngle = ui->editYAngle->value();
    auto zAngle = ui->editZAngle->value();
    return QVector3D(xAngle, yAngle, zAngle);
}

void ExamEditDialog::setAngle(QVector3D other) {
    if (other == this->angle()) {
        return;
    }

    ui->editXAngle->setValue(other.x());
    ui->editYAngle->setValue(other.y());
    ui->editZAngle->setValue(other.z());
}

void ExamEditDialog::setSlices(QJsonArray slicesArray) {
    auto sliceNum = slicesArray.count();

    setSliceComboNumbers(sliceNum);

    m_slices.resize(sliceNum);

    for (int i = 0; i < sliceNum; i++) {
        auto slice = slicesArray[i].toObject();

        QVector3D angles(slice[KEY_X_ANGLE].toDouble(),
                         slice[KEY_Y_ANGLE].toDouble(),
                         slice[KEY_Z_ANGLE].toDouble());

        QVector3D offsets(slice[KEY_X_OFFSET].toDouble(),
                          slice[KEY_Y_OFFSET].toDouble(),
                          slice[KEY_Z_OFFSET].toDouble());

        auto sliceData = std::make_shared<SliceData>(angles, offsets);
        m_slices[i] = sliceData;
    }

    setSliceComboNumbers(slicesArray.count());
}

QJsonArray ExamEditDialog::jsonSlices() {
    QJsonArray out;
    for (const auto &slice : m_slices) {
        QJsonObject item;
        item[KEY_X_ANGLE] = slice->angle().x();
        item[KEY_Y_ANGLE] = slice->angle().y();
        item[KEY_Z_ANGLE] = slice->angle().z();
        item[KEY_X_OFFSET] = slice->offset().x();
        item[KEY_Y_OFFSET] = slice->offset().y();
        item[KEY_Z_OFFSET] = slice->offset().z();
        out.append(item);
    }
    return out;
}

void ExamEditDialog::setSliceComboNumbers(int n) {
    if (n < 0)
        return;

    const QSignalBlocker blocker(ui->comboSlice);
    ui->comboSlice->clear();

    QStringList numbers;
    numbers.reserve(n);

    for (int i = 0; i < n; i++) {
        numbers << QString::number(i);
    }

    ui->comboSlice->addItems(numbers);

    if (n > 0) {
        ui->comboSlice->setCurrentIndex(0);
    }
}

void ExamEditDialog::on_comboSlice_currentIndexChanged(int index) {
    const auto &curSlice = m_slices[index];

    ui->editXAngle->setValue(curSlice->angle().x());
    ui->editYAngle->setValue(curSlice->angle().y());
    ui->editZAngle->setValue(curSlice->angle().z());
    ui->editXOffset->setValue(curSlice->offset().x());
    ui->editYOffset->setValue(curSlice->offset().y());
    ui->editZOffset->setValue(curSlice->offset().z());
}
