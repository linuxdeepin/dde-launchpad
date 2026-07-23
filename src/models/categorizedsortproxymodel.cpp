// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "categorizedsortproxymodel.h"

#include <QGuiApplication>
#include <QSet>
#include <QLoggingCategory>

#include <DConfig>

Q_DECLARE_LOGGING_CATEGORY(logModels)

DCORE_USE_NAMESPACE

void CategorizedSortProxyModel::setCategoryType(CategoryType categoryType)
{
    CategoryType oldCategoryType = this->categoryType();

    // Temporarily disable dynamic sort to prevent setSortRole from triggering
    // a redundant sort. We trigger a single sort below via sort(0),
    // which uses layoutAboutToBeChanged/layoutChanged instead of modelReset,
    // preserving delegates.
    const bool wasDynamic = dynamicSortFilter();
    setDynamicSortFilter(false);
    isFreeSort = (categoryType == FreeCategory);
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

    if (oldCategoryType != categoryType) {
        QScopedPointer<DConfig> config(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.launchpad"));
        config->setValue("categoryType", categoryType);
    }

    // Use sort(0) instead of setDynamicSortFilter(true) because the latter
    // calls d->sort() without setting proxy_sort_column, leaving it at -1
    // (the Qt 6 default). When source_sort_column is -1,
    // QSortFilterProxyModelPrivate::sort_source_rows falls through to
    // std::less{} (a no-op), so no sorting actually occurs.
    // sort(0) properly sets proxy_sort_column = 0 and calls
    // update_source_sort_column(), then d->sort() emits
    // layoutAboutToBeChanged/layoutChanged so the view moves existing
    // delegates instead of destroying and recreating them.
    sort(0);
    setDynamicSortFilter(wasDynamic);

    // Must update sectionRoleName after the sort so that the QML ListView
    // evaluates section structure with the correct (already sorted) item order.
    m_sectionRoleName = sortRoleName();
    emit sectionRoleNameChanged();

    qCInfo(logModels) << "Category type changed to:" << categoryType;
    emit categoryTypeChanged();
}

CategorizedSortProxyModel::CategoryType CategorizedSortProxyModel::categoryType() const
{
    if (isFreeSort) return FreeCategory;
    if (sortRole() == AppsModel::TransliteratedRole) return CategorizedSortProxyModel::Alphabetary;
    return CategorizedSortProxyModel::DDECategory;
}

QString CategorizedSortProxyModel::sortRoleName() const
{
    return QString(sourceModel()->roleNames().value(sortRole()));
}

QList<QString> CategorizedSortProxyModel::alphabetarySections() const
{
    // If ‘&’ or ‘#’ appears, put it at the front. If they appear at the same time, ‘&’ will put it at the front.
    auto customLessThan = [](const QString &s1, const QString &s2) {
        if (s1 == "&" && s2 == "#")
            return true;
        else if (s1 == "#" && s2 == "&")
            return false;
        else
            return s1 < s2;
    };

    QSet<QString> charset;
    for (int i = 0; i < rowCount(); i++) {
        QString transliterated = data(index(i, 0), AppsModel::TransliteratedRole).toString();
        if (!transliterated.isEmpty()) {
            charset.insert(transliterated.constData()[0].toUpper());
        }
    }

    QList<QString> values = charset.values();
    std::sort(values.begin(), values.end(), customLessThan);

    return values;
}

QList<int> CategorizedSortProxyModel::DDECategorySections() const
{
    QSet<int> ddeCategorySet;

    for (int i = 0; i < rowCount(); i++) {
        auto value = data(index(i, 0), AppItem::DDECategoryRole);
        if (value.isValid()) {
            ddeCategorySet.insert(value.toInt());
        }
    }

    // 排序，以便和显示顺序一致
    QList<int> values = ddeCategorySet.values();
    std::sort(values.begin(), values.end());

    return values;
}

bool CategorizedSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (sortRole() == AppsModel::TransliteratedRole) {
        QString l_transliterated = source_left.model()->data(source_left, sortRole()).toString();
        QString r_transliterated = source_right.model()->data(source_right, sortRole()).toString();
        QChar l_start = l_transliterated.isEmpty() ? QChar() : l_transliterated.constData()[0].toUpper();
        QChar r_start = r_transliterated.isEmpty() ? QChar() : r_transliterated.constData()[0].toUpper();
        if (l_start != r_start) {
            return l_start < r_start;
        } else {
            QString l_display = source_left.model()->data(source_left, Qt::DisplayRole).toString();
            QString r_display = source_right.model()->data(source_right, Qt::DisplayRole).toString();
            QChar ld_start = l_display.isEmpty() ? QChar() : l_display.constData()[0].toUpper();
            QChar rd_start = r_display.isEmpty() ? QChar() : r_display.constData()[0].toUpper();
            if ((l_start == ld_start && ld_start == rd_start) || (l_start != ld_start && l_start != rd_start)) {
                // display name both start with ascii letter, or both NOT start with ascii letter
                // use their transliterated form for sorting
                if (!l_start.isNull() && l_transliterated.constData()[0] != r_transliterated.constData()[0]) {
                    // Since in ascii table, `A` is lower than `a`, we specially check to ensure `a` is lower here.
                    return l_transliterated.constData()[0].isLower();
                }
                return l_transliterated < r_transliterated;
            } else {
                // one of them are ascii letter and another of them is non-ascii letter.
                // the ascii one should be display on the front
                return l_start == ld_start;
            }
        }
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

CategorizedSortProxyModel::CategorizedSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(&AppsModel::instance());
    QScopedPointer<DConfig> config(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.launchpad"));
    CategoryType categoryType = CategoryType(config->value("categoryType", FreeCategory).toInt());
    isFreeSort = (categoryType == FreeCategory);
    setCategoryType(categoryType);
    qCDebug(logModels) << "CategoryType by DConfig:" << categoryType;
}
