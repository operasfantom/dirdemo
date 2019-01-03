#include "directory_controller.h"

directory_controller::directory_controller(QObject *parent) : QObject(parent) {}

void directory_controller::set_directory(const QString &directory_name)
{
    directory = QDir(directory_name);
    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    directory.setSorting(QDir::Size | QDir::Reversed);
}

void directory_controller::scan_directory(std::function<void (const QFileInfoList &)> callback, bool sync)
{
    if (sync) {
        repo.scan_directory(directory, callback);
        emit finished();
    } else {
        if (scan_thread.joinable()) {
            scan_thread.join();
        }
        scan_thread = std::thread([this, callback]() {
            try {
                repo.scan_directory(directory, callback);
                emit finished();
            } catch (std::exception const& e) {
                emit finished();
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
