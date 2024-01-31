// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcherappiconprovider.h"

#include "iconutils.h"

LauncherAppIconProvider::LauncherAppIconProvider():
    QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

LauncherAppIconProvider::~LauncherAppIconProvider()
{

}

QPixmap LauncherAppIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)

    QSize preferredSize = requestedSize.isValid()
                              ? requestedSize
                              : ((size && size->isValid()) ? *size : QSize(48, 48));

    QPixmap result(preferredSize);
    result.fill(Qt::transparent);

    // uri: image://provider/icon-theme-name/icon-name
    // id: icon-name
    QString iconName = id;
    const int iconThemeIndex = id.indexOf("/");
    if (iconThemeIndex >= 0)
        iconName = id.mid(iconThemeIndex + 1);

    IconUtils::getThemeIcon(result, iconName, preferredSize.width());

    return result;
}
