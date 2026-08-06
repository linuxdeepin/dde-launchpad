// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheritem.h"
#include "pluginfactory.h"
#include "../launchercontroller.h"
#include <blurhashimageprovider.h>
#include <appitem.h>
#include <appsmodel.h>
#include <itemarrangementproxymodel.h>

#include <DDBusSender>
#include <QLoggingCategory>

#include <applet.h>
#include <qmlengine.h>

DS_USE_NAMESPACE
namespace {
Q_LOGGING_CATEGORY(logApplet, "org.deepin.dde.launchpad.applet")

constexpr QLatin1StringView groupIdPrefix("internal/folders/");

int folderIdFromGroupId(const QString &groupId)
{
    if (!groupId.startsWith(groupIdPrefix))
        return -1;

    bool ok = false;
    const int folderId = QStringView(groupId).sliced(groupIdPrefix.size()).toInt(&ok);
    return ok && folderId > 0 ? folderId : -1;
}
}

namespace dock {

LauncherItem::LauncherItem(QObject *parent)
    : DApplet(parent)
    , m_iconName("deepin-launcher")
{
    qCDebug(logApplet) << "Initializing LauncherItem applet";
}

bool LauncherItem::init()
{
    DApplet::init();

    DQmlEngine().engine()->addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
        !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
        qCWarning(logApplet) << "Register D-Bus service failed";
    } 
    qCInfo(logApplet) << "LauncherItem initialization completed";
    return true;
}

QAbstractItemModel *LauncherItem::itemArrangementModel() const
{
    return &ItemArrangementProxyModel::instance();
}

QString LauncherItem::groupDisplayName(const QString &groupId) const
{
    if (folderIdFromGroupId(groupId) < 0)
        return {};

    auto &model = ItemArrangementProxyModel::instance();
    const auto matches = model.match(model.index(0, 0), AppItem::DesktopIdRole,
                                     groupId, 1, Qt::MatchExactly);
    if (matches.isEmpty())
        return {};

    return matches.constFirst().data(Qt::DisplayRole).toString();
}

QVariantList LauncherItem::groupItemDetails(const QString &groupId) const
{
    const int folderId = folderIdFromGroupId(groupId);
    if (folderId < 0)
        return {};

    const auto desktopIds = ItemArrangementProxyModel::instance().folderItems(folderId);
    QVariantList details;
    details.reserve(desktopIds.size());
    for (const auto &desktopId : desktopIds) {
        const auto item = AppsModel::instance().itemFromDesktopId(desktopId);
        if (!item)
            continue;

        details.append(QVariantMap{
            {QStringLiteral("desktopId"), desktopId},
            {QStringLiteral("name"), item->name()},
            {QStringLiteral("iconName"), item->iconName()},
        });
    }
    return details;
}

D_APPLET_CLASS(LauncherItem)
}


#include "launcheritem.moc"
