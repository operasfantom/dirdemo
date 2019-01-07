#ifndef SCANNINGPROGRESS_H
#define SCANNINGPROGRESS_H

#include <QDialog>

namespace Ui {
class ScanningProgress;
}

class ScanningProgress : public QDialog
{
    Q_OBJECT

public:
    explicit ScanningProgress(QWidget *parent = nullptr);
    ~ScanningProgress();

signals:
    void cancel();
private slots:
    void on_cancelButton_clicked();

private:
    Ui::ScanningProgress *ui;
};

#endif // SCANNINGPROGRESS_H
