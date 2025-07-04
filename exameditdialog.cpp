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

    connect(ui->editFOV, &QDoubleSpinBox::valueChanged, this, [this](double fov) {
        ui->scoutWidget->setSlicesFov(fov);
    });

    connect(ui->editSliceSeparation, &QDoubleSpinBox::valueChanged, this,
            [this](double separation) {
                ui->scoutWidget->setSeparation(separation);
            });

    connect(ui->editNoSlices, &QSpinBox::valueChanged, this, &ExamEditDialog::onNoSlicesChanged);
}

void ExamEditDialog::setData(const Exam &exam) {
    QJsonObject parameters = exam.request().params();

    ui->editFOV->setValue(parameters[KEY_FOV].toDouble());
    ui->editNoAverages->setValue(parameters[KEY_NO_AVERAGES].toInt());
    ui->editNoSamples->setValue(parameters[KEY_NO_SAMPLES].toInt());
    ui->editNoViews->setValue(parameters[KEY_NO_VIEWS].toInt());
    ui->editObserveFrequency->setValue(
        parameters[KEY_OBSERVE_FREQUENCY].toDouble());
    ui->editSliceThickness->setValue(parameters[KEY_SLICE_THICKNESS].toDouble());
    ui->editNoSlices->setValue(parameters[KEY_NO_SLICES].toInt());

    if (parameters.contains(KEY_SLICES)) {
        ui->checkGroupMode->setChecked(false);
        setSlices(parameters[KEY_SLICES].toArray());
        ui->stackedWidget->setCurrentIndex(0);
        ui->comboSlice->setCurrentIndex(0);
    } else {
        ui->checkGroupMode->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
        
        auto angle = QVector3D(parameters[KEY_X_ANGLE].toDouble(), parameters[KEY_Y_ANGLE].toDouble(), parameters[KEY_Z_ANGLE].toDouble());
        auto offset = QVector3D(parameters[KEY_X_OFFSET].toDouble(), parameters[KEY_Y_OFFSET].toDouble(), parameters[KEY_Z_OFFSET].toDouble());
        auto slice = makeSlice(angle, offset);

        setSeparation(parameters[KEY_SLICE_SEPARATION].toDouble());
        setSlice(0, slice);

        ui->scoutWidget->setSlices({m_slices[0]});
        ui->scoutWidget->setNoSlices(parameters[KEY_NO_SLICES].toInt());  // 必须在checkGroupMode为true时设置才会正确触发绘制
    }

    ui->scoutWidget->updateMarkers();
}

void ExamEditDialog::setSlice(int index, std::shared_ptr<SliceData> slice) {
    disconnect(m_slices[index].get(), &SliceData::angleChanged, this, nullptr);
    disconnect(m_slices[index].get(), &SliceData::offsetChanged, this, nullptr);

    m_slices[index] = slice;

    connect(m_slices[index].get(), &SliceData::angleChanged, this,
            [this, index](QVector3D angle) {
                if (index == ui->comboSlice->currentIndex()) {
                    this->setAngle(angle);
                }
            });
    connect(m_slices[index].get(), &SliceData::offsetChanged, this,
            [this, index](QVector3D offset) {
                if (index == ui->comboSlice->currentIndex()) {
                    this->setOffset(offset);
                }
            });
}

void ExamEditDialog::setSeparation(double separation) {
    ui->editSliceSeparation->setValue(separation);
}

void ExamEditDialog::setNoSlices(int noSlices) {
    ui->editNoSlices->setValue(noSlices);
}

void ExamEditDialog::clear() {
    ui->editFOV->setValue(0);
    ui->editNoAverages->setValue(0);
    ui->editNoSlices->setValue(0);
    ui->editNoSamples->setValue(0);
    ui->editNoViews->setValue(0);

    setSliceComboNumbers(0);
    m_slices.clear();
    ui->scoutWidget->setSlices({});
    setSeparation(0);
    setNoSlices(0);
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

    if (ui->checkGroupMode->isChecked()) {
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

        auto sliceData = makeSlice(angles, offsets);
        m_slices[i] = sliceData;

    }

    ui->scoutWidget->setSlices({m_slices[0]});
}

std::shared_ptr<SliceData> ExamEditDialog::makeSlice(QVector3D angles, QVector3D offsets) {
    auto sliceData = std::make_shared<SliceData>(angles, offsets);

    auto index = ui->comboSlice->currentIndex();
    connect(sliceData.get(), &SliceData::angleChanged, this,
            [this, index](QVector3D angle) {
                if (index == ui->comboSlice->currentIndex()) {
                    this->setAngle(angle);
                }
            });
    connect(sliceData.get(), &SliceData::offsetChanged, this,
            [this, index](QVector3D offset) {
                if (index == ui->comboSlice->currentIndex()) {
                    this->setOffset(offset);
                }
            });
    return sliceData;
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

    ui->scoutWidget->setSlices({curSlice});
}

void ExamEditDialog::onNoSlicesChanged(int num)
{
    // group mode无视m_slices
    // 只扩容，不缩容
    if (ui->checkGroupMode->isChecked()) {
        ui->scoutWidget->setNoSlices(num);
        return;
    }else{
        for (int i = m_slices.count(); i < num; i++) {
            m_slices.push_back(makeSlice(QVector3D(0, 0, 0), QVector3D(0, 0, 0)));
        }
        setSliceComboNumbers(num);
    }
}
