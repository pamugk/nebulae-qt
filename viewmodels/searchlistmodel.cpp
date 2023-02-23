#include "searchlistmodel.h"

SearchListModel::SearchListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant SearchListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

int SearchListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return 0;
}

bool SearchListModel::hasChildren(const QModelIndex &parent) const
{
    return false;
}

bool SearchListModel::canFetchMore(const QModelIndex &parent) const
{
    // FIXME: Implement me!
    return false;
}

void SearchListModel::fetchMore(const QModelIndex &parent)
{
    // FIXME: Implement me!
}

QVariant SearchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
