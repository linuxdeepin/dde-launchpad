// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmgr.h"

#include "AppManager1Application.h"

using AppManager1Application = __AppManager1Application;

AppMgr::AppMgr(QObject *parent)
    : QObject(parent)
{

}

AppMgr::~AppMgr()
{

}

inline QString escapeToObjectPath(const QString &str)
{
    if (str.isEmpty()) {
        return "_";
    }

    auto ret = str;
    static QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(_%1)"}.arg(hexStr));
        }
    }
    return ret;
}

// if return false, it means the launch is not even started.
// if return true, it means we attempted to launch it via AM, but not sure if it's succeed.
bool AppMgr::launchApp(const QString &desktopId)
{
    // the new dde-application-manager use systemd-style Application ID, which is
    // basicly the freedesktop desktop-id sins the ".desktop" suffix.
    constexpr int suffixLen = std::char_traits<char>::length(".desktop");
    QString systemdAppId(desktopId.chopped(suffixLen));
    QString dbusPath = QString("/org/desktopspec/ApplicationManager1/%1").arg(escapeToObjectPath(systemdAppId));

    AppManager1Application * amAppIface = new AppManager1Application(QLatin1String("org.desktopspec.ApplicationManager1"),
                                                                    dbusPath,
                                                                    QDBusConnection::sessionBus());
    if (!amAppIface->isValid()) {
        qDebug() << "D-Bus interface not exist or failed to connect to" << dbusPath;
        return false;
    }

    amAppIface->Launch(QString(), QStringList{}, QVariantMap());

    return true;
}
