// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appwiz.h"

#include "DaemonLauncher1.h"

using DaemonLauncher1 = __DaemonLauncher1;

AppWiz::AppWiz(QObject *parent)
    : QObject(parent)
    , m_dbusDaemonLauncherIface(new DaemonLauncher1(DaemonLauncher1::staticInterfaceName(), "/org/deepin/dde/daemon/Launcher1",
                                                    QDBusConnection::sessionBus(), this))
{

}

AppWiz::~AppWiz()
{

}

// TODO: remove this and the whole m_dbusDaemonLauncherIface thing once the legacy dde-launcher is gone.
void AppWiz::legacyRequestUninstall(const QString &desktopFileFullPath)
{
    qDebug() << "uninstall" << desktopFileFullPath;
    qDebug() << m_dbusDaemonLauncherIface->lastError();

    QDBusPendingReply rpy = m_dbusDaemonLauncherIface->RequestUninstall(desktopFileFullPath, false);
    if (rpy.isError()) {
        qDebug() << rpy.error();
    }
}
