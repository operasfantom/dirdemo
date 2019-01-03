#ifndef DIRECTORY_CONTROLLER_H
#define DIRECTORY_CONTROLLER_H

#include <QFileInfoList>
#include <QDir>

#include <thread>
#include <functional>

#include "repository.h"

namespace Ui {
class directory_controller;
}

class directory_controller : public QObject
{
    Q_OBJECT
private:
    QDir directory;

    repository repo;

    std::thread scan_thread;
public:
    explicit directory_controller(QObject *parent = 0);

    virtual ~directory_controller() = default;

    void set_directory(QString const& directory_name);

    void scan_directory(std::function<void(const QFileInfoList &file_info_list)> callback, bool sync);

    void remove_duplicates();

    void cancel_scanning();
signals:
    void finished();
};

#endif // DIRECTORY_CONTROLLER_H
