#include <QDir>
#include <QDirIterator>
#include <QVector>
#include <QFile>

#include <memory>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cstring>

#include "repository.h"


bool repository::equals(const QFileInfo &file_info1, const QFileInfo &file_info2)
{
    QFile file1(file_info1.absoluteFilePath());
    QFile file2(file_info2.absoluteFilePath());
    if (file1.open(QIODevice::ReadOnly) && file2.open(QIODevice::ReadOnly)) {
        while (!file1.atEnd()) {
            file1.read(buffer1.data(), BUFFER_SIZE);
            file2.read(buffer2.data(), BUFFER_SIZE);
            if (std::memcmp(buffer1.data(), buffer2.data(), BUFFER_SIZE) != 0) {
                return false;
            }
        }
        return true;
    } else {
        throw std::runtime_error("Could not open file");
    }
}

void repository::clear_storage()
{
    file_by_size.clear();
    clusters.clear();
    buffer1.clear();
    buffer2.clear();
}

repository::repository()
{
    buffer1.resize(BUFFER_SIZE);
    buffer2.resize(BUFFER_SIZE);
}

void repository::scan_directory(const QDir &directory)
{
    clear_storage();
    if (directory.exists()) {
        state = State::IN_PROCESS;
        QDirIterator it(directory.path(), QDir::Files, QDirIterator::Subdirectories);
        while (state == State::IN_PROCESS && it.hasNext()) {
            QFileInfo info(it.next());
            file_by_size[info.size()].push_back(info);
        }
        for (auto const&it : file_by_size) {
            if (state != State::IN_PROCESS) {
                break;
            }
            auto const& list = it.second;
            int size = list.size();
            QVector<int> file_id(size, -1);

            int identities = 0;
            for (int i = 0; i < size; ++i) {
                if (list[i].exists() && list[i].isReadable()) {
                    for (int j = 0; j < i; ++j) {
                        if (file_id[j] >= 0) {
                            try {
                                if (equals(list[i], list[j])) {
                                    file_id[i] = file_id[j];
                                    break;
                                }
                            } catch (std::runtime_error const& e) {
                                //todo
                            }
                        }
                    }
                    if (file_id[i] == -1) {
                        file_id[i] = identities++;
                    }
                } else {
                    file_id[i] = -2;
                }
            }
            QVector<QFileInfoList> buckets(identities);
            for (int i = 0; i < size; ++i) {
                if (file_id[i] >= 0) {
                    buckets[file_id[i]].push_back(list[i]);
                }
            }
            for (auto && bucket : buckets) {
                if (bucket.size() > 1) {
                    clusters.push_back(std::move(bucket));
                }
            }
        }
        if (state == State::IN_PROCESS) {
            state = State::COMPLETED;
        }
        if (state == State::COMPLETED) {
            for (auto const& cluster : clusters) {
                emit callback(&cluster);
            }
            return;
        }
        if (state == State::CANCELLED) {
            //todo
            return;
        }
    }
}

void repository::remove_duplicates()
{
    for (auto const &it : clusters) {
        for (auto const &file_info : it) {
            QFile file(file_info.absoluteFilePath());
            if (!file.remove()) {
                //todo
            }
        }
    }
    clear_storage();
}
