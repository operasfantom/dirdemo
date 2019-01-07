#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QFileInfoList>

#include <memory>
#include <thread>

#include "directory_controller.h"
#include "scanningprogress.h"

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

private slots:
    void select_directory();
    void scan_directory(QString directory_name);
    void show_about_dialog();
    void remove_duplicates();
    void cancel_scanning();
    void close_progress_dialog(bool success);
    void show_duplicates_group(QFileInfoList file_info_list);

private:
//    std::unique_ptr<Ui::MainWindow> ui;
    Ui::MainWindow* ui;

    ScanningProgress * progress_dialog;

    directory_controller controller;
};

#endif // MAINWINDOW_H