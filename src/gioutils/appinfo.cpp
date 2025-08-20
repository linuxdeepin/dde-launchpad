// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appinfo.h"

#undef signals
#include <gio/gdesktopappinfo.h>
#define signals Q_SIGNALS

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(logGioUtils, "dde.launchpad.gioutils")
}


// return value indicates if a desktopId can be match to a runnable GAppInfo. NOT the result of the launch action
bool AppInfo::launchByDesktopId(const QString &desktopId)
{
    qCDebug(logGioUtils) << "Attempting to launch app with desktopId:" << desktopId;
    
    GDesktopAppInfo * appInfo = g_desktop_app_info_new(desktopId.toStdString().c_str());
    if (!appInfo) {
        qCWarning(logGioUtils) << "Failed to create app info for desktopId:" << desktopId;
        return false;
    }

    qCInfo(logGioUtils) << "Launching application with desktopId:" << desktopId;
    g_app_info_launch((GAppInfo*)appInfo, NULL, NULL, NULL);

    return true;
}

QString AppInfo::fullPathByDesktopId(const QString &desktopId)
{
    qCDebug(logGioUtils) << "Getting full path for desktopId:" << desktopId;
    
    GDesktopAppInfo * appInfo = g_desktop_app_info_new(desktopId.toStdString().c_str());
    if (!appInfo) {
        qCWarning(logGioUtils) << "Failed to create app info for desktopId:" << desktopId;
        return QString();
    }

    const char * filePath = g_desktop_app_info_get_filename(appInfo);
    const auto& result = QString::fromUtf8(filePath);
    
    qCDebug(logGioUtils) << "Full path for desktopId" << desktopId << "is:" << result;
    return result;
}
