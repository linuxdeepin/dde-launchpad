// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheritem.h"
#include "pluginfactory.h"
#include "../launchercontroller.h"
#include <appsmodel.h>
#include <blurhashimageprovider.h>

#include <QLoggingCategory>
#include <QDBusConnection>

#include <applet.h>
#include <appletbridge.h>
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

bool LauncherItem::load()
{
    DAppletBridge bridge(QStringLiteral("org.deepin.ds.dde-apps"));
    auto applet = bridge.applet();
    if (!applet) {
        qCWarning(logApplet) << "Unable to load launchpad without the dde-apps applet";
        return false;
    }

    auto model = applet->property("appModel").value<QAbstractItemModel *>();
    if (!model) {
        qCWarning(logApplet) << "dde-apps did not provide an application model";
        return false;
    }

    auto &appsModel = AppsModel::instance();
    appsModel.setSourceModel(model);
    appsModel.setReady(applet->property("appModelReady").toBool());
    appsModel.setDdeCategories(applet->property("ddeCategories").toMap());
    connect(applet, SIGNAL(appModelReadyChanged(bool)), &appsModel, SLOT(setReady(bool)));

    qCInfo(logApplet) << "Using dde-shell shared application model";
    return DApplet::load();
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
