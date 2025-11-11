// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheritem.h"
#include "pluginfactory.h"
#include "../launchercontroller.h"
#include <blurhashimageprovider.h>

#include <DDBusSender>
#include <QLoggingCategory>

#include <applet.h>
#include <qmlengine.h>

DS_USE_NAMESPACE
namespace {
Q_LOGGING_CATEGORY(logApplet, "org.deepin.dde.launchpad.applet")
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

D_APPLET_CLASS(LauncherItem)
}


#include "launcheritem.moc"
