// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "freesortproxymodel.h"
#include "itemarrangementproxymodel.h"

FreeSortProxyModel::FreeSortProxyModel(QObject *parent)
    : SortProxyModel(parent)
{
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