// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frequentlyusedproxymodel.h"

#include <QDebug>
#include <DConfig>
DCORE_USE_NAMESPACE

FrequentlyUsedProxyModel::FrequentlyUsedProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    QScopedPointer<DConfig> dconfig(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.launchpad"));
    // lower priority is higher.
    m_frequentlyUsedAppIdList = dconfig->value("frequentlyUsedAppIdList").toStringList();
    qDebug() << "Fetched frequentlyUsed app list by DConfig" << m_frequentlyUsedAppIdList;
    std::reverse(m_frequentlyUsedAppIdList.begin(), m_frequentlyUsedAppIdList.end());

    connect(this, &QAbstractProxyModel::sourceModelChanged, this, [=](){
        sort(0, Qt::DescendingOrder);
    });
}

bool FrequentlyUsedProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    if (inRecentlyInstalledModel(modelIndex))
        return false;

    return true;
}

bool FrequentlyUsedProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    const auto leftLaunchedTimes = sourceLeft.data(m_launchedTimesRole).toLongLong();
    const auto rightLaunchedTimes = sourceRight.data(m_launchedTimesRole).toLongLong();

    if (leftLaunchedTimes != rightLaunchedTimes)
        return leftLaunchedTimes < rightLaunchedTimes;

    // Only compare lastLaunchedTime when LaunchedTimes is not zero (
    // maybe the app is auto-start instead of by-user).
    if (leftLaunchedTimes != 0 && rightLaunchedTimes != 0) {
        const auto leftLastLaunchedTime = sourceLeft.data(m_lastLaunchedTimeRole).toLongLong();
        const auto rightLastLaunchedTimes = sourceRight.data(m_lastLaunchedTimeRole).toLongLong();
        if (leftLastLaunchedTime != rightLastLaunchedTimes)
            return leftLastLaunchedTime < rightLastLaunchedTimes;
    }

    return lessThenByFrequentlyUsed(sourceLeft, sourceRight);
}

QAbstractItemModel *FrequentlyUsedProxyModel::recentlyInstalledModel() const
{
    return m_recentlyInstalledModel;
}

void FrequentlyUsedProxyModel::setRecentlyInstalledModel(QAbstractItemModel *newRecentlyInstalledModel)
{
    if (m_recentlyInstalledModel == newRecentlyInstalledModel)
        return;

    static const struct {
        const char *signalName;
        const char *slotName;
    } connectionTable[] = {
        { SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(invalidate()) },
        { SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(invalidate()) },
        { SIGNAL(layoutChanged()), SLOT(invalidate()) },
    };

    if (m_recentlyInstalledModel) {
        for (const auto &c : connectionTable)
            disconnect(m_recentlyInstalledModel, c.signalName, this, c.slotName);
    }

    m_recentlyInstalledModel = newRecentlyInstalledModel;

    if (m_recentlyInstalledModel) {
        for (const auto &c : connectionTable) {
            connect(m_recentlyInstalledModel, c.signalName, this, c.slotName);
        }
    }
    invalidate();

    emit recentlyInstalledModelChanged();
}

void FrequentlyUsedProxyModel::classBegin()
{
}

void FrequentlyUsedProxyModel::componentComplete()
{
    sort(0, Qt::DescendingOrder);
}

bool FrequentlyUsedProxyModel::inRecentlyInstalledModel(const QModelIndex &index) const
{
    if (!m_recentlyInstalledModel)
        return false;

    const auto desktopId = index.data(m_desktopIdRole).toString();

    const auto targetModel = m_recentlyInstalledModel;
    for (int i = 0; i < targetModel->rowCount(); i++) {
        const auto id = targetModel->data(targetModel->index(i, 0), m_desktopIdRole).toString();
        if (id == desktopId)
            return true;
    }
    return false;
}

bool FrequentlyUsedProxyModel::lessThenByFrequentlyUsed(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    const auto leftId = sourceLeft.data(m_desktopIdRole).toString();
    const auto rightId = sourceRight.data(m_desktopIdRole).toString();
    const auto leftInFrequentlyUsed = m_frequentlyUsedAppIdList.indexOf(leftId);
    const auto rightInFrequentlyUsed = m_frequentlyUsedAppIdList.indexOf(rightId);
    return leftInFrequentlyUsed < rightInFrequentlyUsed;
}
