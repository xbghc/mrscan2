#ifndef TUNINGSHIMMING_H
#define TUNINGSHIMMING_H

#include <QDialog>

namespace Ui {
class TuningShimming;
}

class TuningShimming : public QDialog
{
    Q_OBJECT

public:
    explicit TuningShimming(QWidget *parent = nullptr);
    ~TuningShimming();

private:
    Ui::TuningShimming *ui;
};

#endif // TUNINGSHIMMING_H
