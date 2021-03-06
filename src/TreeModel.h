﻿#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QFileInfoList>
#include <QStringList>

class TreeItem;

class TreeModel : public QAbstractItemModel {
Q_OBJECT

public:
	TreeModel(const QStringList& headers, QObject* parent = 0);
	~TreeModel();

	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
	                    int role = Qt::DisplayRole) const override;

	QModelIndex index(int row, int column,
	                  const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value,
	             int role = Qt::EditRole) override;
	bool setHeaderData(int section, Qt::Orientation orientation,
	                   const QVariant& value, int role = Qt::EditRole) override;

	bool insertColumns(int position, int columns,
	                   const QModelIndex& parent = QModelIndex()) override;
	bool removeColumns(int position, int columns,
	                   const QModelIndex& parent = QModelIndex()) override;
	bool insertRows(int position, int rows,
	                const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int position, int rows,
	                const QModelIndex& parent = QModelIndex()) override;
	void clear();

	void addGroup(QFileInfoList list);
	QString fileName(int row) const;
	bool isValidRow(int i) const;
	bool isChecked(int row) const;
	QStringList checkedItems() const;
	void hideFiles(QSet<QString>);
public slots:
	void sortByColumn(int column, Qt::SortOrder order);
private:
	void sort(int column, Qt::SortOrder order) override;
	TreeItem* getItem(const QModelIndex& index) const;

	TreeItem* rootItem;

};

#endif // TREEMODEL_H
