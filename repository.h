#ifndef SCANNER_H
#define SCANNER_H

#include <QFileInfoList>
#include <QString>

#include <functional>
#include <unordered_map>

class repository
{
    std::unordered_map<qint64, QFileInfoList> map;
public:
    repository() = default;

    void scan_directory(QDir const& directory, std::function<void(const QFileInfoList &file_info_list)> callback);

    void remove_duplicates();
};

#endif // SCANNER_H
