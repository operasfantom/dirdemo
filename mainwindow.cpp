#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDirIterator>
#include <QPushButton>

#include <memory>
#include <thread>

#include "repository.h"
#include "scanningprogress.h"

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);

    connect(&controller, SIGNAL(send_duplicates_group(const QFileInfoList *)), this, SLOT(show_duplicates_group(const QFileInfoList *))/*, Qt::BlockingQueuedConnection*/);
//    scan_directory(QDir::homePath());
}

main_window::~main_window()
{}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    scan_directory(dir);
}


void main_window::scan_directory(QString const& directory_name)
{
    ui->treeWidget->clear();
    setWindowTitle(QString("Directory Content - %1").arg(directory_name));

    progress_dialog = new ScanningProgress();//std::make_unique<ScanningProgress>();
    progress_dialog->show();
    connect(progress_dialog, SIGNAL(cancel()), this, SLOT(cancel_scanning())/*, Qt::BlockingQueuedConnection*/);
    connect(&controller, SIGNAL(finished(bool)), this, SLOT(close_progress_dialog(bool)), Qt::BlockingQueuedConnection);

    controller.set_directory(directory_name);

    controller.scan_directory(false);
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}

void main_window::remove_duplicates()
{
    controller.remove_duplicates();
}

void main_window::cancel_scanning()
{
    controller.cancel_scanning();
}

void main_window::close_progress_dialog(bool success)
{
    progress_dialog->close();
}

void main_window::show_duplicates_group(const QFileInfoList *file_info_list)
{
    QList<QTreeWidgetItem*> items;
    items.reserve(file_info_list->size());
    std::transform(file_info_list->begin(), file_info_list->end(), items.begin(), [this](QFileInfo const& file_info){
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, file_info.absoluteFilePath());
        item->setText(1, QString::number(file_info.size()));
        item->setCheckState(2, Qt::CheckState::Unchecked);
        item->checkState(2);

        return item;
    });

    items.push_back(new QTreeWidgetItem());//fake separator

    ui->treeWidget->addTopLevelItems(items);
}
