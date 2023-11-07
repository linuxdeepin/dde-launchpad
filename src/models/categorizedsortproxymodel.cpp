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

CategorizedSortProxyModel::CategorizedSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(&AppsModel::instance());
}
