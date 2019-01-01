#ifndef DIRECTORY_CONTROLLER_H
#define DIRECTORY_CONTROLLER_H

#include <QFileInfoList>
#include <QDir>

#include <thread>
#include <functional>

#include "repository.h"

class directory_controller
{
    QDir directory;

    repository repo;

    std::thread scan_thread;

public:
    directory_controller() = default;

    void set_directory(QString const& directory_name);

    void scan_directory(std::function<void(const QFileInfoList &file_info_list)> callback, bool sync);

    void remove_duplicates();
};

#endif // DIRECTORY_CONTROLLER_H
