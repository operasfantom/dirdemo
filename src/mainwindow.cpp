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

void main_window::finished(bool success)
{
    ui->cancelButton->setEnabled(false);
}

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry(this)));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    qRegisterMetaType<QFileInfoList>("QFileInfoList");

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->actionRemove_Duplicates, &QAction::triggered, this, &main_window::remove_duplicates);

    connect(&controller, SIGNAL(send_duplicates_group(QFileInfoList)), this, SLOT(show_duplicates_group(QFileInfoList)));
    connect(&controller, &directory_controller::finished, this, &main_window::finished);
    connect(&controller, &directory_controller::set_progress, ui->progressBar, &QProgressBar::setValue);
//    scan_directory(QDir::homePath());
}

main_window::~main_window()
{
    delete ui;
}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    scan_directory(dir);
}


void main_window::scan_directory(QString directory_name)
{
    ui->cancelButton->setEnabled(true);

    ui->treeWidget->clear();
    setWindowTitle(QString("Directory Content - %1").arg(directory_name));

    controller.set_directory(directory_name);

    controller.scan_directory(false);
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}

void main_window::remove_duplicates()
{
    QFileInfoList files;
    for (auto item : ui->treeWidget->findItems(
             QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive)) {
        if (item->columnCount() == 3 && item->checkState(2) == Qt::CheckState::Checked) {
            files.push_back(QFileInfo(item->text(0)));
            item->setHidden(true);
        }
    }
    controller.remove_files(files);
}

void main_window::cancel_scanning()
{
    controller.cancel_scanning();
    finished(false);
}

void main_window::show_duplicates_group(QFileInfoList file_info_list)
{
    QList<QTreeWidgetItem*> items;
    items.reserve(file_info_list.size());
    std::transform(file_info_list.begin(), file_info_list.end(), items.begin(), [this](QFileInfo file_info){
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, file_info.absoluteFilePath());
        item->setText(1, QString::number(file_info.size()));
        item->setCheckState(2, Qt::CheckState::Unchecked);        

        return item;
    });

    items.push_back(new QTreeWidgetItem());//fake separator

    ui->treeWidget->addTopLevelItems(items);   
}

void main_window::on_cancelButton_clicked()
{
    cancel_scanning();
}
