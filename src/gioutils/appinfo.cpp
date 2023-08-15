// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appinfo.h"

#undef signals
#include <gio/gdesktopappinfo.h>
#define signals Q_SIGNALS


// return value indicates if a desktopId can be match to a runnable GAppInfo. NOT the result of the launch action
bool AppInfo::launchByDesktopId(const QString &desktopId)
{
    GDesktopAppInfo * appInfo = g_desktop_app_info_new(desktopId.toStdString().c_str());
    if (!appInfo) return false;

    g_app_info_launch((GAppInfo*)appInfo, NULL, NULL, NULL);

    return true;
}

QString AppInfo::fullPathByDesktopId(const QString &desktopId)
{
    GDesktopAppInfo * appInfo = g_desktop_app_info_new(desktopId.toStdString().c_str());
    if (!appInfo) return QString();

    const char * filePath = g_desktop_app_info_get_filename(appInfo);

    return QString::fromUtf8(filePath);
}
