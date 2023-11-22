// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multipagesortfilterproxymodel.h"

#include <QDebug>

#include "multipageproxymodel.h"

MultipageSortFilterProxyModel::MultipageSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(MultipageProxyModel::FolderIdNumberRole);
    setDynamicSortFilter(true);

    connect(this, &MultipageSortFilterProxyModel::onFolderIdChanged, this, [this](){
        invalidateFilter();
    });

    connect(this, &MultipageSortFilterProxyModel::onPageIdChanged, this, [this](){
        invalidateFilter();
    });
}

MultipageSortFilterProxyModel::~MultipageSortFilterProxyModel()
{

}

void MultipageSortFilterProxyModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }
    QSortFilterProxyModel::setSourceModel(model);
    sort(0);
}

bool MultipageSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), MultipageProxyModel::FolderIdNumberRole).toInt() == m_folderId &&
           sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), MultipageProxyModel::PageRole).toInt() == m_pageId;
}

bool MultipageSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (source_left.data(MultipageProxyModel::FolderIdNumberRole).toInt() < source_right.data(MultipageProxyModel::FolderIdNumberRole).toInt()) {
        return true;
    } else if (source_left.data(MultipageProxyModel::PageRole).toInt() < source_right.data(MultipageProxyModel::PageRole).toInt()) {
        return true;
    } else if (source_left.data(MultipageProxyModel::IndexInPageRole).toInt() < source_right.data(MultipageProxyModel::IndexInPageRole).toInt()) {
        return true;
    } else {
        return false;
    }
}
