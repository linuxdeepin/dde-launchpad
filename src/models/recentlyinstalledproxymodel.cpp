// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentlyinstalledproxymodel.h"
#include "appitem.h"
#include "appsmodel.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logModels)


RecentlyInstalledProxyModel::RecentlyInstalledProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    qCDebug(logModels) << "Initializing RecentlyInstalledProxyModel";
}

bool RecentlyInstalledProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    if (!modelIndex.isValid())
        return false;
    const auto lastLaunchedTime = modelIndex.data(m_lastLaunchedTimeRole).toLongLong();
    if (lastLaunchedTime > 0)
        return false;

    // filter pre installed applications.
    const auto installedTime = modelIndex.data(m_installedTimeRole).toLongLong();
    return installedTime > 0;
}

bool RecentlyInstalledProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (!source_left.isValid() || !source_right.isValid())
        return false;

    const auto leftTime = source_left.data(m_installedTimeRole).toLongLong();
    const auto rightTime = source_right.data(m_installedTimeRole).toLongLong();

    return leftTime < rightTime;
}

void RecentlyInstalledProxyModel::classBegin()
{
    qCDebug(logModels) << "RecentlyInstalledProxyModel class begin";
}

void RecentlyInstalledProxyModel::componentComplete()
{
    sort(0, Qt::DescendingOrder);
}
