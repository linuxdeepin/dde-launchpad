// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"

AppItem::AppItem(const QString &freedesktopId)
    : QStandardItem()
{
    setData(freedesktopId, AppItem::DesktopIdRole);
}

AppItem::~AppItem()
{

}

const QString AppItem::freedesktopId() const
{
    return data(AppItem::DesktopIdRole).toString();
}

const QString AppItem::displayName() const
{
    return text();
}

void AppItem::setDisplayName(const QString &name)
{
    setText(name);
}

const QString AppItem::iconName() const
{
    return data(AppItem::IconNameRole).toString();
}

void AppItem::setIconName(const QString &iconName)
{
    setData(iconName, AppItem::IconNameRole);
}

const QStringList AppItem::categories() const
{
    return data(AppItem::Categories).toStringList();
}

void AppItem::setCategories(const QStringList &categories)
{
    setData(categories, AppItem::Categories);
}

AppItem::DDECategories AppItem::ddeCategory() const
{
    return data(AppItem::DDECategoryRole).value<AppItem::DDECategories>();
}

void AppItem::setDDECategory(DDECategories category)
{
    setData(category, AppItem::DDECategoryRole);
}

qulonglong AppItem::installedTime() const
{
    return data(AppItem::InstalledTimeRole).toULongLong();
}

void AppItem::setInstalledTime(qulonglong time)
{
    setData(time, AppItem::InstalledTimeRole);
}

qulonglong AppItem::lastLaunchedTime() const
{
    return data(AppItem::LastLaunchedTimeRole).toULongLong();
}

void AppItem::setLastLaunchedTime(qulonglong time)
{
    setData(time, AppItem::LastLaunchedTimeRole);
}

// assign/update data from another AppItem object
// assume the desktopId is the same, will update other data.
// doesn't take the ownership of the passed appItem.
void AppItem::updateData(const AppItem *appItem)
{
    if (this == appItem) return;
    setData(appItem->data(Qt::DisplayRole), Qt::DisplayRole);
    setData(appItem->data(AppItem::IconNameRole), AppItem::IconNameRole);
    setData(appItem->data(AppItem::Categories), AppItem::Categories);
    setData(appItem->data(AppItem::DDECategoryRole), AppItem::DDECategoryRole);
    setInstalledTime(appItem->installedTime());
    setLastLaunchedTime(appItem->lastLaunchedTime());
}

