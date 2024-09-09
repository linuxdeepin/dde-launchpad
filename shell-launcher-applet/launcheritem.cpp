// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheritem.h"
#include "pluginfactory.h"
#include "../launchercontroller.h"

#include <DDBusSender>

#include <applet.h>

namespace dock {

LauncherItem::LauncherItem(QObject *parent)
    : DApplet(parent)
    , m_iconName("deepin-launcher")
{

}

bool LauncherItem::init()
{
    DApplet::init();

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
        !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
        qWarning() << "register dbus service failed";
    }

    return true;
}

D_APPLET_CLASS(LauncherItem)
}


#include "launcheritem.moc"
