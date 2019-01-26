#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QMetaType>

#include <thread>

void main_window::finished(bool success) {
	ui->cancelButton->setEnabled(false);
	ui->treeView->setSortingEnabled(true);
	connect(ui->treeView->header(), &QHeaderView::sortIndicatorChanged, treeModel, &TreeModel::sortByColumn);
}

void main_window::show_message(QString message, Status status) {
	ui->messageLog->appendPlainText(to_string(status) + ":" + message);
}

Q_DECLARE_METATYPE(QFileInfoList);

main_window::main_window(QWidget* parent)
	: QMainWindow(parent)
	  , ui(new Ui::MainWindow), treeModel(new TreeModel(QStringList{"File name", "Size", "Remove?"})) {
	ui->setupUi(this);
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
	                                qApp->desktop()->availableGeometry(this)));

	ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	// ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	// ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	QCommonStyle style;
	ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
	ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
	ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
	
	qRegisterMetaType<QFileInfoList>("QFileInfoList");

	connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
	connect(ui->actionRemove_Duplicates, &QAction::triggered, this, &main_window::remove_duplicates);

	connect(&controller, SIGNAL(send_duplicates_group(QFileInfoList)), this, SLOT(show_duplicates_group(QFileInfoList)), Qt::BlockingQueuedConnection);
	connect(&controller, &directory_controller::finished, this, &main_window::finished);
	connect(&controller, &directory_controller::set_progress, ui->progressBar, &QProgressBar::setValue);

	connect(&controller, &directory_controller::caused_error, [this](QString message) {
		show_message(message, ERROR);
	});
	//    scan_directory(QDir::homePath());
	ui->treeView->setModel(treeModel);
}

main_window::~main_window() {
	delete ui;
}

void main_window::select_directory() {
	QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
	                                                QString(),
	                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()) {
		scan_directory(dir);
	}
}


void main_window::scan_directory(QString directory_name) {
	ui->cancelButton->setEnabled(true);
	ui->treeView->setSortingEnabled(false);

	treeModel->clear();
	setWindowTitle(QString("Directory Content - %1").arg(directory_name));

	controller.set_directory(directory_name);

	controller.scan_directory(false);
}

void main_window::show_about_dialog() {
	QMessageBox::aboutQt(this);
}

void main_window::remove_duplicates() {
	QSet<QString> removed_files;
	for (auto file_name : treeModel->checkedItems()) {
		if (controller.remove_file(file_name)) {
			removed_files.insert(file_name);
		}
		else {
			show_message("couldn't remove file: " + file_name, ERROR);
		}
	}
	treeModel->hideFiles(removed_files);
}

void main_window::cancel_scanning() {
	controller.cancel_scanning();
	show_message("scanning was cancelled", WARN);
	finished(false);
}

void main_window::show_duplicates_group(QFileInfoList file_info_list) {
	treeModel->addGroup(file_info_list);
}

void main_window::on_cancelButton_clicked() {
	cancel_scanning();
}
