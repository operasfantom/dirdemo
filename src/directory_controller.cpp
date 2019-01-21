#include "directory_controller.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QDirIterator>

void directory_controller::clear_storage() {
	files_by_size.clear();
	files_by_hash.clear();
	clusters.clear();
}

void directory_controller::group_by_size() {
	QDirIterator it(directory.path(), QDir::Files, QDirIterator::Subdirectories);
	while (state == State::IN_PROCESS && it.hasNext()) {
		QFileInfo file_info(it.next());
		files_by_size[file_info.size()].push_back(file_info);
		++files_count;
	}
	int i = 0;
	std::for_each(files_by_size.begin(), files_by_size.end(), [this](QFileInfoList const& list) {
		if (list.size() == 1) {
			unique.insert(list[0].absoluteFilePath());
		}
	});
}

void directory_controller::group_by_hash() {
	QDirIterator it(directory.path(), QDir::Files, QDirIterator::Subdirectories);

	int i = 0;
	while (state == State::IN_PROCESS && it.hasNext()) {
		QFileInfo file_info(it.next());
		if (!unique.contains(file_info.absoluteFilePath())) {
			QFile file(file_info.absoluteFilePath());
			try {
				if (file.open(QFile::ReadOnly)) {
					QCryptographicHash hash(QCryptographicHash::Md4);
					while (file.read(buffer.data(), BUFFER_SIZE) != 0 && state == State::IN_PROCESS) {
						hash.addData(buffer);
					}
					files_by_hash[hash.result()].push_back(file_info);
				}
				else {
					emit caused_error("couldn't open file for ReadOnly mode:" + file_info.absoluteFilePath());
				}
			}
			catch (...) { }
		}
		emit set_progress(100 * (++i) / files_count);
	}
}

void directory_controller::scan_directory0() {
	clear_storage();
	emit set_progress(0);
	if (directory.exists()) {
		state = State::IN_PROCESS;
		group_by_size();
		group_by_hash();
		if (state == State::IN_PROCESS) {
			for (auto&& duplicate_group : files_by_hash) {
				if (duplicate_group.size() > 1) {
					clusters.push_back(std::move(duplicate_group));
					emit send_duplicates_group(clusters.back());
				}
			}
		}
		if (state == State::IN_PROCESS) {
			state = State::COMPLETED;
		}
	}
}

directory_controller::directory_controller(QObject* parent) : QObject(parent), files_count(0) {
	buffer.resize(BUFFER_SIZE);
	files_by_size.reserve(RESERVED_BUCKETS);
	files_by_hash.reserve(RESERVED_BUCKETS);
	clusters.reserve(RESERVED_BUCKETS);
}

void directory_controller::set_directory(QString directory_name) {
	directory = QDir(directory_name);
	directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	directory.setSorting(QDir::Size | QDir::Reversed);

	files_count = 0;
}

void directory_controller::scan_directory(bool sync) {
	auto f = [this]() {
		try {
			scan_directory0();
			emit finished(true);
		}
		catch (...) {
			emit finished(false);
		}
	};
	if (sync) {
		f();
	}
	else {
		scan_thread = QThread::create(f);
		scan_thread->start();
	}
}

bool directory_controller::remove_file(QString file_name) {
	QStringList removed_files;
	QFile file(file_name);
	return file.remove();
}

void directory_controller::cancel_scanning() {
	state = State::CANCELLED;
}
