#include <QtWidgets>
#include <QCheckBox>
#include <QDebug>

#include "treeitem.h"
#include "treemodel.h"


TreeModel::TreeModel(const QStringList& headers, QObject* parent)
	: QAbstractItemModel(parent) {
	QVector<QVariant> rootData;
	foreach(QString header, headers)
		rootData << header;

	rootItem = new TreeItem(rootData);
	// setupModelData(data.split(QString("\n")), rootItem);
}

TreeModel::~TreeModel() {
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex& /* parent */) const {
	return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();
	TreeItem* item = getItem(index);
	if (role == Qt::CheckStateRole && index.column() == 2 && isValidRow(index.row())) {
		return static_cast<int>(item->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	if (role != Qt::DisplayRole && role != Qt::EditRole) {
		return QVariant();
	}
	return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;
	if (index.column() == 2) {
		return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
	}

	return QAbstractItemModel::flags(index);;
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const {
	if (index.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TreeItem* parentItem = getItem(parent);

	TreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex& parent) {
	bool success;

	beginInsertColumns(parent, position, position + columns - 1);
	success = rootItem->insertColumns(position, columns);
	endInsertColumns();

	return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, rootItem->columnCount());
	endInsertRows();

	return success;
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem* childItem = getItem(index);
	TreeItem* parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex& parent) {
	bool success;

	beginRemoveColumns(parent, position, position + columns - 1);
	success = rootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (rootItem->columnCount() == 0)
		removeRows(0, rowCount());

	return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
	TreeItem* parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

void TreeModel::clear() {
	if (rowCount() > 0) {
		removeRows(0, rowCount());
	}
}

void TreeModel::addGroup(QFileInfoList list) {
	int count = rowCount();
	insertRows(count, list.size() + 1);
	for (int i = 0; i < list.size(); ++i) {
		if (list[i].exists()) {
			setData(index(count + i, 0), list[i].absoluteFilePath());
			setData(index(count + i, 1), list[i].size());
		}
	}
}

QString TreeModel::fileName(int row) const {
	return data(index(row, 0), Qt::DisplayRole).toString();
}

bool TreeModel::isValidRow(int i) const {
	return data(this->index(i, 0), Qt::DisplayRole).isValid();
}

bool TreeModel::isChecked(int row) const {
	return getItem(index(row, 2))->isChecked();
}

QStringList TreeModel::checkedItems() const {
	QStringList res;
	for (int i = 0; i < rowCount(); ++i) {
		if (isValidRow(i) && isChecked(i)) {
			res.append(fileName(i));
		}
	}
	return res;
}

void TreeModel::hideFiles(QSet<QString> set) {
	QVector<int> rows;
	for (int i = 0; i < rowCount(); ++i) {
		if (isValidRow(i)) {
			if (isChecked(i)) {
				if (set.contains(data(index(i, 0), Qt::DisplayRole).toString())) {
					rows.append(i);
				}
			}
		}
	}
	for (int i = 0; i < rows.size(); ++i) {
		removeRows(rows[i] - i, 1);
	}
}

void TreeModel::sortByColumn(int column, Qt::SortOrder order) {
	sort(column, order);
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void TreeModel::sort(int column, Qt::SortOrder order) {
	using P = QPair<QString, qint64>;
	QVector<QVector<P>> res;
	QVector<P> t;
	for (int i = 0; i < rowCount(); ++i) {
		QVariant file_name(data(index(i, 0), Qt::EditRole));
		QVariant size(data(index(i, 1), Qt::EditRole));
		if (file_name.isValid()) {
			t.append(P(file_name.value<QString>(), size.value<qint64>()));
		}
		else {
			res.append(std::move(t));
			t.clear();
		}
	}
	switch (column) {
	case 0:
		for (auto& chunk : res) {
			std::sort(chunk.begin(), chunk.end(), [](P const& x, P const& y) {
				return x.first < y.first;
			});
			if (order == Qt::DescendingOrder) {
				std::reverse(chunk.begin(), chunk.end());
			}
		}
		break;
	case 1:
		std::sort(res.begin(), res.end(), [](QVector<P> const& x, QVector<P> const& y) {
			return x[0].second < y[0].second;
		});
		if (order == Qt::DescendingOrder) {
			std::reverse(res.begin(), res.end());
		}
		break;
	case 2:
		break;
	}
	clear();
	for (auto& chunk : res) {
		QFileInfoList list;
		list.reserve(chunk.size());
		for (auto& p : chunk) {
			list.append(QFileInfo(p.first));
		}
		addGroup(list);
	}
}

int TreeModel::rowCount(const QModelIndex& parent) const {
	TreeItem* parentItem = getItem(parent);

	return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (role != Qt::EditRole && role != Qt::CheckStateRole)
		return false;
	TreeItem* item = getItem(index);
	if (role == Qt::CheckStateRole && index.column() == 2) {
		int val = value.toInt();
		bool value = (static_cast<Qt::CheckState>(val) == Qt::Checked);
		item->setChecked(value);
		return value;
	}

	bool result = item->setData(index.column(), value);

	if (result) {
		emit dataChanged(index, index);
	}

	return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant& value, int role) {
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
		return false;

	bool result = rootItem->setData(section, value);

	if (result) {
		emit headerDataChanged(orientation, section, section);
	}

	return result;
}
