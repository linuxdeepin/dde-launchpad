// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcherfoldericonprovider.h"

#include "iconutils.h"

#include <QPainter>
#include <QGuiApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logQuick)

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

    QSize preferredSize = requestedSize.isValid()
                              ? requestedSize
                              : ((size && size->isValid()) ? *size : QSize(64, 64));
    const auto [iconSize, padding] = IconUtils::getFolderPerfectIconCell(preferredSize.width(), iconPerRow);
    int iconSpacing = padding;
    qCDebug(logQuick) << "Calculated icon size:" << iconSize << "padding:" << padding;
    QPixmap result(preferredSize);
    result.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&result);

    qreal radius = 12.0 * qGuiApp->devicePixelRatio();

    // folder background
    painter.setBrush(QBrush(QColor(255, 255, 255, 255 * 0.15)));
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundedRect(result.rect(), radius, radius);

    // icons
    // uri: image://provider/icon-name:icon-name:icon-name
    // ids: icon-name:icon-name:icon-name
    const QStringList ids(id.split(':'));
    int curIdx = 0;
    for (const QString & icon : ids) {
        int curRow = curIdx / iconPerRow;
        int curCol = curIdx % iconPerRow;
        QPixmap iconPixmap(QSize(iconSize, iconSize));
        iconPixmap.fill(Qt::transparent);
        IconUtils::getThemeIcon(iconPixmap, icon, iconSize);
        QRect iconRect;
        iconRect.setTop(padding + curRow * (iconSize + iconSpacing));
        iconRect.setLeft(padding + curCol * (iconSize + iconSpacing));
        iconRect.setSize(QSize(iconSize, iconSize));
        painter.drawPixmap(iconRect, iconPixmap);
        curIdx++;
    }

    painter.end();
    qCDebug(logQuick) << "Folder icon pixmap created successfully, size:" << result.size();
    return result;
}
