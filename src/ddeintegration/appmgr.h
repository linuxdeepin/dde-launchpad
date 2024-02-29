// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QObject>
#include <QPointer>

class __AppManager1Application;
class __AppManager1ApplicationObjectManager;
class AppMgr : public QObject
{
    Q_OBJECT
public:
    explicit AppMgr(QObject *parent = nullptr);
    ~AppMgr();
    struct AppItem
    {
        QPointer<__AppManager1Application> handler;
        QString id;
        QString name;
        QString displayName;
        QString iconName;
        QStringList categories;
        qint64 installedTime = 0;
        qint64 lastLaunchedTime = 0;
        qint64 launchedTimes = 0;
    };

    static AppMgr *instance();

    static bool launchApp(const QString & desktopId);
    static bool autoStart(const QString & desktopId);
    static void setAutoStart(const QString & desktopId, bool autoStart);
    static double scaleFactor(const QString & desktopId);
    static void setScaleFactor(const QString & desktopId, double scaleFactor);
    static bool isOnDesktop(const QString & desktopId);
    static bool sendToDesktop(const QString & desktopId);
    static bool removeFromDesktop(const QString & desktopId);

    bool isValid() const;
    QList<AppMgr::AppItem *> allAppInfosShouldBeShown() const;
    AppMgr::AppItem * appItem(const QString &id) const;

Q_SIGNALS:
    void changed();
    void itemDataChanged(const QString &id);

private:
    void initObjectManager();
    void fetchAppItems();
    void watchingAppItemAdded(const QString &key, AppMgr::AppItem *appItem);
    void watchingAppItemRemoved(const QString &key);
    void watchingAppItemPropertyChanged(const QString &key, AppMgr::AppItem *appItem);

private:
    __AppManager1ApplicationObjectManager *m_objectManager;
    QMap<QString, AppMgr::AppItem *> m_appItems;
};
