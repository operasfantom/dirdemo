#ifndef TST_TEST_H
#define TST_TEST_H

#include <QtTest>
#include <QCoreApplication>

#include "../src/directory_controller.h"

class test : public QObject
{
    Q_OBJECT

public:
    test();
    ~test();
private:
    const QDir CURRENT_PATH = QDir::currentPath();
    const QString GENERATE_FOLDER = "GENERATE_PATH";
    QDir GENERATE_PATH = CURRENT_PATH.filePath(GENERATE_FOLDER);

    directory_controller controller;

    QSet<int> group_sizes;

    void create_file(QString data, int quantity = 1);
    void create_file_subdirectory(QString data, int quantity = 1);
private slots:
    void init();
    void cleanup();
    void test_case_empty_folder();
    void test_case1();
    void test_case2();
    void test_large_group();
    void test_sub_directory();
};

#endif // TST_TEST_H
