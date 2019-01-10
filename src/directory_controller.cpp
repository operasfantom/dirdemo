#include "directory_controller.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QDirIterator>

void directory_controller::clear_storage()
{
    files_by_size.clear();
    files_by_hash.clear();
    clusters.clear();
}

void directory_controller::group_by_size()
{
    QDirIterator it(directory.path(), QDir::Files, QDirIterator::Subdirectories);
    while (state == State::IN_PROCESS && it.hasNext()) {
        QFileInfo file_info(it.next());
        files_by_size[file_info.size()].push_back(file_info);
    }
    for (auto it = files_by_size.begin(); it != files_by_size.end(); ++it) {
        if (it.value().size() == 1) {
            unique.insert(it.value()[0].absoluteFilePath());
        }
    }
}

void directory_controller::group_by_hash()
{
    QDirIterator it(directory.path(), QDir::Files, QDirIterator::Subdirectories);
    while (state == State::IN_PROCESS && it.hasNext()) {
        QFileInfo file_info(it.next());
        if (!unique.contains(file_info.absoluteFilePath())){
            QFile file(file_info.absoluteFilePath());
            try {
                if (file.open(QFile::ReadOnly)) {
                    QCryptographicHash hash(QCryptographicHash::Sha256);
                    hash.addData(&file);
                    files_by_hash[hash.result()].push_back(file_info);
                }
                file.close();
            } catch (...) {
                file.close();
            }
        }
    }
}

void directory_controller::scan_directory0()
{
    clear_storage();
    if (directory.exists()) {
        state = State::IN_PROCESS;
        group_by_size();
        group_by_hash();
        if (state == State::IN_PROCESS) {
            for (auto &&duplicate_group : files_by_hash) {
                if (duplicate_group.size() > 1) {
                    clusters.push_back(std::move(duplicate_group));
                    emit send_duplicates_group(clusters.back());
                }
            }
        }
        if (state == State::IN_PROCESS) {
            state = State::COMPLETED;
        }
        if (state == State::COMPLETED) {
            return;
        }
        if (state == State::CANCELLED) {
            //todo
            return;
        }
    }
}

directory_controller::directory_controller(QObject *parent) : QObject(parent) {
    buffer.resize(BUFFER_SIZE);
    files_by_size.reserve(RESERVED_BUCKETS);
    files_by_hash.reserve(RESERVED_BUCKETS);
    clusters.reserve(RESERVED_BUCKETS);
}

void directory_controller::set_directory(QString directory_name)
{
    directory = QDir(directory_name);
    directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    directory.setSorting(QDir::Size | QDir::Reversed);
}

void directory_controller::scan_directory(bool sync)
{
    if (sync) {
        try {
            scan_directory0();
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
                scan_directory0();
                emit finished(true);
            } catch (...) {
                emit finished(false);
            }
        });
    }
}

void directory_controller::remove_files(QFileInfoList files)
{
    for (auto const &file_info : files) {
        QFile file(file_info.absoluteFilePath());
        if (!file.remove()) {
            //todo
        }
    }
}

void directory_controller::cancel_scanning()
{
    state = State::CANCELLED;
}
