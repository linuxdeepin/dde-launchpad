// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcherfoldericonprovider.h"

#include "iconutils.h"

#include <QPainter>

LauncherFolderIconProvider::LauncherFolderIconProvider():
    QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

LauncherFolderIconProvider::~LauncherFolderIconProvider()
{

}

QPixmap LauncherFolderIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)

    constexpr int iconPerRow = 2;
    constexpr int iconSpacing = 5;
    constexpr int padding = 5;

    QSize preferredSize = requestedSize.isValid()
                              ? requestedSize
                              : ((size && size->isValid()) ? *size : QSize(64, 64));
    const int iconSize = (preferredSize.width() - padding * 2 - (iconPerRow - 1) * iconSpacing) / iconPerRow;

    QPixmap result(preferredSize);
    result.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&result);

    // folder background
    painter.setBrush(QBrush(QColor(0, 0, 0, 128)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(result.rect(), 8.0, 8.0);

    // icons
    // uri: image://provider/icon-name:icon-name:icon-name
    // ids: icon-name:icon-name:icon-name
    const QStringList ids(id.split(':'));
    int curIdx = 0;
    for (const QString & icon : ids) {
        int curRow = curIdx / iconPerRow;
        int curCol = curIdx % iconPerRow;
        QPixmap iconPixmap(QSize(iconSize, iconSize));
        IconUtils::getThemeIcon(iconPixmap, icon, iconSize);
        QRect iconRect;
        iconRect.setTop(padding + curRow * (iconSize + iconSpacing));
        iconRect.setLeft(padding + curCol * (iconSize + iconSpacing));
        iconRect.setSize(QSize(iconSize, iconSize));
        painter.drawPixmap(iconRect, iconPixmap);
        curIdx++;
    }

    painter.end();

    return result;
}
