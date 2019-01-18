#ifndef DIRECTORY_CONTROLLER_H
#define DIRECTORY_CONTROLLER_H

#include <QFileInfoList>
#include <QDir>
#include <QFileInfoList>
#include <QVector>
#include <QSet>

#include <thread>
#include <functional>

class directory_controller : public QObject
{
    Q_OBJECT
private:
    enum class State {
        CANCELLED,
        IN_PROCESS,
        COMPLETED
    };

    QDir directory;

    int files_count = 0;

    QThread* scan_thread;

    std::atomic<State> state{State::COMPLETED};

    QHash<qint64, QFileInfoList> files_by_size;

    QSet<QString> unique;

    QHash<QByteArray, QFileInfoList> files_by_hash;

    QVector<QFileInfoList> clusters;

    const int BUFFER_SIZE = 4 * 1024 * 1024;

    const int RESERVED_BUCKETS = 100 * 1000;//todo increase

    QByteArray buffer;

    void clear_storage();

    void group_by_size();

    void group_by_hash();

    void scan_directory0();
public:

    explicit directory_controller(QObject *parent = nullptr);

    virtual ~directory_controller() = default;

    void set_directory(QString directory_name);

    void scan_directory(bool sync);

    void remove_files(QFileInfoList files);

    void cancel_scanning();
signals:
    void finished(bool success);

    void send_duplicates_group(QFileInfoList file_info_list);

    void set_progress(int value);
};

#endif // DIRECTORY_CONTROLLER_H
