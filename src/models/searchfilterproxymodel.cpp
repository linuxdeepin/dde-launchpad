// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "searchfilterproxymodel.h"

#include <QDebug>
#include <DPinyin>
DCORE_USE_NAMESPACE

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

    const QString & displayName = modelIndex.data(Qt::DisplayRole).toString();
    const QString & name = modelIndex.data(AppsModel::NameRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::AllTransliteratedRole).toString();
    const QString & jianpin = Dtk::Core::firstLetters(displayName).join(',');

    auto nameCopy = name;
    nameCopy = nameCopy.toLower();
    nameCopy.replace(" ", "");

    return displayName.contains(searchPattern) || nameCopy.contains(searchPattern) || transliterated.contains(searchPattern) || jianpin.contains(searchPattern);
}
