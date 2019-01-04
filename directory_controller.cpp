#include "directory_controller.h"

directory_controller::directory_controller(QObject *parent) : QObject(parent) {
    connect(&repo, SIGNAL(callback(const QFileInfoList *)), this, SLOT(receive_duplicates_group(const QFileInfoList *))/*, Qt::BlockingQueuedConnection*/);
}

void directory_controller::set_directory(const QString &directory_name)
{
    directory = QDir(directory_name);
    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    directory.setSorting(QDir::Size | QDir::Reversed);
}

void directory_controller::scan_directory(bool sync)
{
    if (sync) {
        try {
            repo.scan_directory(directory);
            emit finished(true);
        } catch (...) {
            emit finished(false);
        }
    } else {
        if (scan_thread.joinable()) {
            scan_thread.join();
        }
        scan_thread = std::thread([this]() {
            try {
                repo.scan_directory(directory);
                emit finished(true);
            } catch (...) {
                emit finished(false);
            }
        });
    }
}

void directory_controller::remove_duplicates()
{
    repo.remove_duplicates();
}

void directory_controller::cancel_scanning()
{
    repo.state = State::CANCELLED;
}

void directory_controller::receive_duplicates_group(const QFileInfoList *file_info_list)
{
    emit send_duplicates_group(file_info_list);
}
