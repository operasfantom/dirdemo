#ifndef SCANNER_H
#define SCANNER_H

#include <QFileInfoList>
#include <QString>
#include <QVector>

#include <functional>
#include <unordered_map>

enum class State {
    NOT_STARTED,
    CANCELLED,
    IN_PROCESS,
    COMPLETED
};

class repository : public QObject
{
    Q_OBJECT
private:
    friend class directory_controller;

    std::atomic<State> state{State::NOT_STARTED};

    std::unordered_map<qint64, QFileInfoList> file_by_size;
    QVector<QFileInfoList> clusters;

    static const int BUFFER_SIZE = 4 * 1024 * 1024;
    QVector<char> buffer1;
    QVector<char> buffer2;

    bool equals(const QFileInfo &file_info1, const QFileInfo &file_info2);

    void clear_storage();
signals:
    void callback(const QFileInfoList *file_info_list);
public:
    repository();

    void scan_directory(QDir const& directory);

    void remove_duplicates();
};

#endif // SCANNER_H
