#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include <memory>
#include <thread>

#include "directory_controller.h"

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void select_directory();
    void scan_directory(QString const& directory_name);
    void show_about_dialog();

private:
    std::unique_ptr<Ui::MainWindow> ui;

    directory_controller controller;
};

#endif // MAINWINDOW_H
