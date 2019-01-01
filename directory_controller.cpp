#include "directory_controller.h"

void directory_controller::set_directory(const QString &directory_name)
{
    directory = QDir(directory_name);
}

void directory_controller::scan_directory(std::function<void (const QFileInfoList &)> callback, bool sync)
{
    if (sync) {
        repo.scan_directory(directory, callback);
    } else {
        if (scan_thread.joinable()) {
            scan_thread.join();
        }
        scan_thread = std::thread([this, callback]() {
           repo.scan_directory(directory, callback);
        });
    }
}

void directory_controller::remove_duplicates()
{
    repo.remove_duplicates();
}
