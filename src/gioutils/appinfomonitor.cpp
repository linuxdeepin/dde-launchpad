// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appinfomonitor.h"

#include <gio/gdesktopappinfo.h>

#include <QHash>

AppInfoMonitor::AppInfoMonitor(QObject *parent)
    : QObject(parent)
    , m_appInfoMonitor(g_app_info_monitor_get())
{
    g_signal_connect(m_appInfoMonitor, "changed", G_CALLBACK(slot_onAppInfoMonitorChanged), this);
}

AppInfoMonitor::~AppInfoMonitor()
{
    g_object_unref(m_appInfoMonitor);
}

QStringList fromGStrV(const char * const * gstr_array)
{    
    QStringList result;

    if (gstr_array == NULL) return result;

    for (unsigned long i = 0; i < g_strv_length((char**)gstr_array); i++) {
        result.append(QString(gstr_array[i]));
    }

    return result;
}

QList<QHash<QString, QString> > AppInfoMonitor::allAppInfosShouldBeShown()
{
    QList<QHash<QString, QString> > results;
    GList * app_infos = g_app_info_get_all();
    g_list_foreach(
        app_infos,
        [](gpointer data, gpointer user_data) {
            QList<QHash<QString, QString> > * result = (QList<QHash<QString, QString> > *)user_data;
            GAppInfo * app_info = (GAppInfo *)data;
            if (g_app_info_should_show(app_info)) {

                GIcon * icon = g_app_info_get_icon(app_info);
                g_autofree gchar * iconCStr = icon ? g_icon_to_string(icon) : NULL;
                QString iconStr(iconCStr ? iconCStr : "");

                g_autofree char * xDeepinVendor = g_desktop_app_info_get_string((GDesktopAppInfo*)app_info, "X-Deepin-Vendor");
                const QString genericName(g_desktop_app_info_get_generic_name((GDesktopAppInfo*)app_info));
                QString displayName(g_app_info_get_display_name(app_info));

                if (xDeepinVendor && (strcmp(xDeepinVendor, "deepin") == 0) && !genericName.isEmpty()) {
                    displayName = genericName;
                }

                const char * const * keywords = g_desktop_app_info_get_keywords((GDesktopAppInfo*)app_info);
                QStringList keywordsList(fromGStrV(keywords));

                result->append({
                    {"name", displayName},
                    {"id", QString(g_app_info_get_id(app_info))},
                    {"filepath", QString(g_desktop_app_info_get_filename((GDesktopAppInfo*)app_info))},
                    {"categories", QString(g_desktop_app_info_get_categories((GDesktopAppInfo*)app_info))},
                    {"keywords", keywordsList.join(';')},
                    {"icon", iconStr}
                });
            }
        },
        &results
    );
    g_list_free(app_infos);

    return results;
}

void AppInfoMonitor::onAppInfoMonitorChanged(GAppInfoMonitor *gappinfomonitor)
{
    Q_UNUSED(gappinfomonitor)

    Q_EMIT changed();
}

void AppInfoMonitor::slot_onAppInfoMonitorChanged(GAppInfoMonitor *gappinfomonitor, gpointer user_data)
{
    AppInfoMonitor * that = reinterpret_cast<AppInfoMonitor*>(user_data);
    that->onAppInfoMonitorChanged(gappinfomonitor);
}
