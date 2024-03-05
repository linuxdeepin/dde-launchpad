// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentlyinstalledproxymodel.h"
#include "appitem.h"
#include "appsmodel.h"

#include <QDateTime>
#include <QDebug>

static int IntervalDays = 7;
RecentlyInstalledProxyModel::RecentlyInstalledProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(&AppsModel::instance());

    sort(0, Qt::DescendingOrder);
}

bool RecentlyInstalledProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    if (!modelIndex.isValid())
        return false;
    const auto lastLaunchedTime = modelIndex.data(AppItem::LastLaunchedTimeRole).toLongLong();
    if (lastLaunchedTime > 0)
        return false;

    const auto installedTime = modelIndex.data(AppItem::InstalledTimeRole).toLongLong();
    const auto currentTime = QDateTime::currentDateTime();
    const auto lastTime = currentTime.addDays(-IntervalDays).toMSecsSinceEpoch();

    return installedTime > lastTime;
}

bool RecentlyInstalledProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (!source_left.isValid() || !source_right.isValid())
        return false;

    int leftTime = source_left.data(AppItem::InstalledTimeRole).toLongLong();
    int rightTime = source_right.data(AppItem::InstalledTimeRole).toLongLong();

    return leftTime < rightTime;
}
