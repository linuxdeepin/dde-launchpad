// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRect>

class DdeDock;
class Appearance;
class DesktopIntegration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Qt::ArrowType dockPosition READ dockPosition NOTIFY dockPositionChanged)
    Q_PROPERTY(QRect dockGeometry READ dockGeometry NOTIFY dockGeometryChanged)
    Q_PROPERTY(QString backgroundUrl READ backgroundUrl NOTIFY backgroundUrlChanged)

public:
    static DesktopIntegration &instance()
    {
        static DesktopIntegration _instance;
        return _instance;
    }

    Q_INVOKABLE static QString currentDE();
    Q_INVOKABLE static void openShutdownScreen();
    Q_INVOKABLE static void openSystemSettings();
    Q_INVOKABLE static void launchByDesktopId(const QString & desktopId);
    Q_INVOKABLE static bool appIsCompulsoryForDesktop(const QString & desktopId);
    // TODO: async get wallpaper?

    Qt::ArrowType dockPosition() const;
    QRect dockGeometry() const;
    QString backgroundUrl() const;

    Q_INVOKABLE bool isDockedApp(const QString & desktopId) const;
    Q_INVOKABLE void sendToDock(const QString & desktopId);
    Q_INVOKABLE void removeFromDock(const QString & desktopId);
    Q_INVOKABLE bool isOnDesktop(const QString & desktopId) const;
    Q_INVOKABLE void sendToDesktop(const QString & desktopId);
    Q_INVOKABLE void removeFromDesktop(const QString & desktopId);
    Q_INVOKABLE bool isAutoStart(const QString & desktopId) const;
    Q_INVOKABLE void setAutoStart(const QString & desktopId, bool on = true);

signals:
    void dockPositionChanged();
    void dockGeometryChanged();
    void backgroundUrlChanged();

private:
    explicit DesktopIntegration(QObject * parent = nullptr);

    DdeDock * m_dockIntegration;
    Appearance * m_appearanceIntegration;
};
