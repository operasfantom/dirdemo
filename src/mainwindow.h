#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QFileInfoList>

#include <memory>
#include <thread>

#include "directory_controller.h"

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

    void finished(bool success);

    void set_progress(int value);
public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

private slots:
    void select_directory();
    void scan_directory(QString directory_name);
    void show_about_dialog();
    void remove_duplicates();
    void cancel_scanning();
    void show_duplicates_group(QFileInfoList file_info_list);

    void on_cancelButton_clicked();

private:
//    std::unique_ptr<Ui::MainWindow> ui;
    Ui::MainWindow* ui;

    directory_controller controller;
};

#endif // MAINWINDOW_H
