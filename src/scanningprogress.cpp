#include "scanningprogress.h"
#include "ui_scanningprogress.h"

ScanningProgress::ScanningProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanningProgress)
{
    ui->setupUi(this);
}

ScanningProgress::~ScanningProgress()
{
    delete ui;
}

void ScanningProgress::on_cancelButton_clicked()
{
    emit cancel();
}
