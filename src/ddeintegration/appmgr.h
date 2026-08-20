// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>

class QTimer;

class AppMgr final : public QObject
{
    Q_OBJECT

public:
    static AppMgr *instance();

    static bool launchApp(const QString &desktopId, const QString &activationToken = {});
    static bool autoStart(const QString &desktopId);
    static void setAutoStart(const QString &desktopId, bool enabled);
    static bool disableScale(const QString &desktopId);
    static void setDisableScale(const QString &desktopId, bool disabled);
    static bool sendToDesktop(const QString &desktopId);
    static bool removeFromDesktop(const QString &desktopId);

    bool waitForIcon(const QString &desktopId, const QString &iconName);
    void cancelPendingAppItem(const QString &desktopId);
    void clearPendingAppItems();
    bool isPendingAppItem(const QString &desktopId) const;

Q_SIGNALS:
    void pendingAppItemReady(const QString &desktopId);

private Q_SLOTS:
    void checkPendingAppItems();

private:
    explicit AppMgr(QObject *parent = nullptr);

    QVariant readProperty(const QString &desktopId, const QString &property) const;
    bool setProperty(const QString &desktopId, const QString &property, const QVariant &value);
    bool callBoolMethod(const QString &desktopId, const QString &method) const;

    QHash<QString, QString> m_pendingAppItems;
    QTimer *m_checkTimer;
    int m_checkCount = 0;
};
