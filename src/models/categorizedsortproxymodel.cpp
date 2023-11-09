// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "categorizedsortproxymodel.h"

#include <QSet>

void CategorizedSortProxyModel::setCategoryType(CategoryType categoryType)
{
    const int oldSortRole = sortRole();

    switch (categoryType) {
    case Alphabetary:
        setSortRole(AppsModel::TransliteratedRole);
        break;
    case DDECategory:
        setSortRole(AppItem::DDECategoryRole);
        break;
    default:
        break;
    }

    if (oldSortRole != sortRole()) {
        emit categoryTypeChanged();
    }

    sort(0);
}

CategorizedSortProxyModel::CategoryType CategorizedSortProxyModel::categoryType() const
{
    if (sortRole() == AppsModel::TransliteratedRole) return CategorizedSortProxyModel::Alphabetary;
    return CategorizedSortProxyModel::DDECategory;
}

QString CategorizedSortProxyModel::sortRoleName() const
{
    return QString(AppsModel::instance().roleNames().value(sortRole()));
}

QList<QString> CategorizedSortProxyModel::alphabetarySections() const
{
    QSet<QString> charset;
    for (int i = 0; i < rowCount(); i++) {
        QString transliterated = data(index(i, 0), AppsModel::TransliteratedRole).toString();
        if (!transliterated.isEmpty()) {
            charset.insert(transliterated.constData()[0].toUpper());
        }
    }

    return charset.values();
}

bool CategorizedSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (sortRole() == AppsModel::TransliteratedRole) {
        QString l_transliterated = source_left.model()->data(source_left, sortRole()).toString();
        QString r_transliterated = source_right.model()->data(source_right, sortRole()).toString();
        QString l_prepend = l_transliterated.isEmpty() ? QString() : l_transliterated.constData()[0].toUpper();
        QString r_prepend = r_transliterated.isEmpty() ? QString() : r_transliterated.constData()[0].toUpper();
        QString l_concat = l_prepend + source_left.model()->data(source_left, Qt::DisplayRole).toString();
        QString r_concat = r_prepend + source_right.model()->data(source_right, Qt::DisplayRole).toString();
        return l_concat < r_concat;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

CategorizedSortProxyModel::CategorizedSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(&AppsModel::instance());
}
