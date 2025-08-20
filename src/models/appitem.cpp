// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"

#include <QFileInfo>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(logModels, "dde.launchpad.models")

AppItem::AppItem(const QString &freedesktopId)
    : QStandardItem()
{
    qCDebug(logModels) << "Creating AppItem:" << freedesktopId;
    setData(freedesktopId, AppItem::DesktopIdRole);
}

AppItem::~AppItem()
{
    qCDebug(logModels) << "Destroying AppItem";
}

const QString AppItem::freedesktopId() const
{
    return data(AppItem::DesktopIdRole).toString();
}

const QString AppItem::name() const
{
    return data(AppItem::NameRole).toString();
}

void AppItem::setName(const QString &name)
{
    setData(name, AppItem::NameRole);
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
    QString name = iconName;
    if (QFileInfo(iconName).isAbsolute()) {
        name = QUrl::fromLocalFile(iconName).toString(); // path ==> file://path
    }

    const auto& finalName = iconName.isEmpty() ? "application-x-desktop" : name;
    qCDebug(logModels) << "Final icon name:" << finalName;
    setData(finalName, AppItem::IconNameRole);
}

const QStringList AppItem::categories() const
{
    return data(AppItem::Categories).toStringList();
}

void AppItem::setCategories(const QStringList &categories)
{
    qCDebug(logModels) << "Setting categories:" << categories;
    setData(categories, AppItem::Categories);
}

AppItem::DDECategories AppItem::ddeCategory() const
{
    return data(AppItem::DDECategoryRole).value<AppItem::DDECategories>();
}

void AppItem::setDDECategory(DDECategories category)
{
    qCDebug(logModels) << "Setting DDE category:" << static_cast<int>(category);
    setData(category, AppItem::DDECategoryRole);
}

qint64 AppItem::installedTime() const
{
    return data(AppItem::InstalledTimeRole).toLongLong();
}

void AppItem::setInstalledTime(qint64 time)
{
    qCDebug(logModels) << "Setting installed time:" << time;
    setData(time, AppItem::InstalledTimeRole);
}

qint64 AppItem::lastLaunchedTime() const
{
    return data(AppItem::LastLaunchedTimeRole).toLongLong();
}

void AppItem::setLastLaunchedTime(qint64 time)
{
    qCDebug(logModels) << "Setting last launched time:" << time;
    setData(time, AppItem::LastLaunchedTimeRole);
}

qint64 AppItem::launchedTimes() const
{
    return data(AppItem::LaunchedTimesRole).toLongLong();
}

void AppItem::setLaunchedTimes(qint64 times)
{
    qCDebug(logModels) << "Setting launched times count:" << times;
    setData(times, AppItem::LaunchedTimesRole);
}

bool AppItem::isAutoStart() const
{
    return data(AppItem::IsAutoStartRole).toBool();
}

void AppItem::setIsAutoStart(bool autostart)
{
    qCDebug(logModels) << "Setting autostart status:" << autostart;
    setData(autostart, AppItem::IsAutoStartRole);
}

const QString AppItem::vendor() const
{
    return data(AppItem::VendorRole).toString();
}

void AppItem::setVendor(const QString &vendor)
{
    qCDebug(logModels) << "Setting vendor:" << vendor;
    setData(vendor, AppItem::VendorRole);
}

const QString AppItem::genericName() const
{
    return data(AppItem::GenericNameRole).toString();
}

void AppItem::setGenericName(const QString &genericName)
{
    qCDebug(logModels) << "Setting generic name:" << genericName;
    setData(genericName, AppItem::GenericNameRole);
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
    setData(appItem->data(AppItem::NameRole), AppItem::NameRole);
    setData(appItem->data(AppItem::InstalledTimeRole), AppItem::InstalledTimeRole);
    setData(appItem->data(AppItem::LastLaunchedTimeRole), AppItem::LastLaunchedTimeRole);
    setData(appItem->data(AppItem::LaunchedTimesRole), AppItem::LaunchedTimesRole);
    setData(appItem->data(AppItem::IsAutoStartRole), AppItem::IsAutoStartRole);
    setData(appItem->data(AppItem::VendorRole), AppItem::VendorRole);
    setData(appItem->data(AppItem::GenericNameRole), AppItem::GenericNameRole);
}

