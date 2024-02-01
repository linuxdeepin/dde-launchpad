// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "favoritedproxymodel.h"
#include "appitem.h"
#include "appsmodel.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

FavoritedProxyModel::FavoritedProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    load();

    setSourceModel(&AppsModel::instance());

    sort(0);
}

bool FavoritedProxyModel::exists(const QString &desktopId)
{
    qDebug() << desktopId << m_favoritedAppIds.contains(desktopId);
    return m_favoritedAppIds.contains(desktopId);
}

void FavoritedProxyModel::addFavorite(const QString &desktopId)
{
    if (m_favoritedAppIds.contains(desktopId)) return;

    m_favoritedAppIds.append(desktopId);

    save();
    invalidate();
}

void FavoritedProxyModel::removeFavorite(const QString &desktopId)
{
    m_favoritedAppIds.removeOne(desktopId);

    save();
    invalidate();
}

void FavoritedProxyModel::pinToTop(const QString &desktopId)
{
    int idx = m_favoritedAppIds.indexOf(desktopId);

    if (idx != -1) {
        m_favoritedAppIds.move(idx, 0);

        save();
        invalidate();
    }
}

bool FavoritedProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);

    return m_favoritedAppIds.contains(modelIndex.data(AppItem::DesktopIdRole).toString());
}

bool FavoritedProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const int leftIndex = m_favoritedAppIds.indexOf(source_left.data(AppItem::DesktopIdRole).toString());
    const int rightIndex = m_favoritedAppIds.indexOf(source_right.data(AppItem::DesktopIdRole).toString());

    return leftIndex < rightIndex;
}

void FavoritedProxyModel::load()
{
    const QString favoriteSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString favoriteSettingPath(QDir(favoriteSettingBasePath).absoluteFilePath("favorited.ini"));
    QSettings favoritedAppsSettings(favoriteSettingPath, QSettings::NativeFormat);

    QStringList predefinedFavoritedApps {
        "deepin-editor.desktop", "deepin-calculator.desktop", "deepin-screen-recorder.desktop", "deepin-terminal.desktop"
    };
    m_favoritedAppIds = favoritedAppsSettings.value("favorited", predefinedFavoritedApps).toStringList();
}

void FavoritedProxyModel::save()
{
    const QString favoriteSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString favoriteSettingPath(QDir(favoriteSettingBasePath).absoluteFilePath("favorited.ini"));
    QSettings favoritedAppsSettings(favoriteSettingPath, QSettings::NativeFormat);

    favoritedAppsSettings.setValue("favorited", m_favoritedAppIds);
}
