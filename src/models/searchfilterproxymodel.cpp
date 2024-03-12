// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "searchfilterproxymodel.h"

#include <QDebug>
#include <DPinyin>

SearchFilterProxyModel::SearchFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    setSourceModel(&AppsModel::instance());
    sort(0, Qt::DescendingOrder);
}

bool SearchFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const QRegularExpression searchPattern = this->filterRegularExpression();

    if (searchPattern.pattern().isEmpty()) {
        if (inRecentlyInstalledModel(modelIndex))
            return false;
    }

    const QString & displayName = modelIndex.data(Qt::DisplayRole).toString();
    const QString & name = modelIndex.data(AppsModel::NameRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::TransliteratedRole).toString();
    const QString & jianpin = Dtk::Core::firstLetters(displayName).join(',');

    auto nameCopy = name;
    nameCopy = nameCopy.toLower();
    nameCopy.replace(" ", "");

    return displayName.contains(searchPattern) || nameCopy.contains(searchPattern) || transliterated.contains(searchPattern) || jianpin.contains(searchPattern);
}

bool SearchFilterProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    const int leftLaunchedTimes = sourceLeft.data(AppItem::LaunchedTimesRole).toInt();
    const int rightLaunchedTimes = sourceRight.data(AppItem::LaunchedTimesRole).toInt();

    return leftLaunchedTimes < rightLaunchedTimes;
}

QAbstractItemModel *SearchFilterProxyModel::recentlyInstalledModel() const
{
    return m_recentlyInstalledModel;
}

void SearchFilterProxyModel::setRecentlyInstalledModel(QAbstractItemModel *newRecentlyInstalledModel)
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

bool SearchFilterProxyModel::inRecentlyInstalledModel(const QModelIndex &index) const
{
    if (!m_recentlyInstalledModel)
        return false;

    const auto desktopId = index.data(AppItem::DesktopIdRole).toString();

    const auto targetModel = m_recentlyInstalledModel;
    for (int i = 0; i < targetModel->rowCount(); i++) {
        const auto id = targetModel->data(targetModel->index(i, 0), AppItem::DesktopIdRole).toString();
        if (id == desktopId)
            return true;
    }
    return false;
}
