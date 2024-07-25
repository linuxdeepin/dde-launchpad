// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multipagesortfilterproxymodel.h"

#include <QDebug>

#include "itemarrangementproxymodel.h"

MultipageSortFilterProxyModel::MultipageSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_filterOnlyMode(false)
{
    setSortRole(ItemArrangementProxyModel::FolderIdNumberRole);
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
    return sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), ItemArrangementProxyModel::FolderIdNumberRole).toInt() == m_folderId &&
           sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), ItemArrangementProxyModel::PageRole).toInt() == m_pageId;
}

bool MultipageSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_filterOnlyMode) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }

    if (source_left.data(ItemArrangementProxyModel::FolderIdNumberRole).toInt() < source_right.data(ItemArrangementProxyModel::FolderIdNumberRole).toInt()) {
        return true;
    } else if (source_left.data(ItemArrangementProxyModel::PageRole).toInt() < source_right.data(ItemArrangementProxyModel::PageRole).toInt()) {
        return true;
    } else if (source_left.data(ItemArrangementProxyModel::IndexInPageRole).toInt() < source_right.data(ItemArrangementProxyModel::IndexInPageRole).toInt()) {
        return true;
    } else {
        return false;
    }
}
