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
}

bool SearchFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const QRegularExpression searchPattern = this->filterRegularExpression();

    const QString & displayName = modelIndex.data(Qt::DisplayRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::TransliteratedRole).toString();
    const QString & jianpin = Dtk::Core::firstLetters(displayName).join(',');

    return displayName.contains(searchPattern) || transliterated.contains(searchPattern) || jianpin.contains(searchPattern);
}
