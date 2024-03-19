// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "freesortproxymodel.h"
#include "itemarrangementproxymodel.h"

FreeSortProxyModel::FreeSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(ItemArrangementProxyModel::FolderIdNumberRole);
    setDynamicSortFilter(true);
}

void FreeSortProxyModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    setSourceModel(model);
    sort(0);
}

bool FreeSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), ItemArrangementProxyModel::FolderIdNumberRole).toInt() == 0;
}

bool FreeSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int leftPage = source_left.data(ItemArrangementProxyModel::PageRole).toInt();
    int rightPage = source_right.data(ItemArrangementProxyModel::PageRole).toInt();

    int leftIndexInPage = source_left.data(ItemArrangementProxyModel::IndexInPageRole).toInt();
    int rightIndexInPage = source_right.data(ItemArrangementProxyModel::IndexInPageRole).toInt();

    if (leftPage == rightPage) {
        return leftIndexInPage < rightIndexInPage;
    } else {
        return leftPage < rightPage;
    }
}