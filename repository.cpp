#include <QDir>
#include <QDirIterator>
#include <QVector>
#include <QFile>

#include <memory>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include "repository.h"

void repository::scan_directory(const QDir &directory, std::function<void(const QFileInfoList &file_info_list)> callback)
{
    if (directory.exists()) {
        QFileInfoList files = directory.entryInfoList();
        std::unordered_map<qint64, QFileInfoList> map;
        std::for_each(files.begin(), files.end(), [&map](QFileInfo const& info){
            map[info.size()].push_back(info);
        });
//        std::sort(files.begin(), files.end(), [](QFileInfo const& p1, QFileInfo const& p2) {
//            return p1.size() < p2.size();
//        });
        for (auto const& it : map) {
            callback(it.second);
        }
    }
}

void repository::remove_duplicates()
{
    for (auto &&it : map) {
        for (auto &&file_info : it.second) {
            QFile file(file_info.path());
            if (!file.remove()) {

            }
        }
    }
}
