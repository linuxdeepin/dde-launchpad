// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRect>
#include <QStandardPaths>
#include <QtQml/qqml.h>

class AppWiz;
class DdeDock;
class Appearance;
class DesktopIntegration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Qt::ArrowType dockPosition READ dockPosition NOTIFY dockPositionChanged)
    Q_PROPERTY(QRect dockGeometry READ dockGeometry NOTIFY dockGeometryChanged)
    Q_PROPERTY(uint dockSpacing READ dockSpacing NOTIFY dockSpacingChanged)
    Q_PROPERTY(QString backgroundUrl READ backgroundUrl NOTIFY backgroundUrlChanged)

    QML_NAMED_ELEMENT(DesktopIntegration)
    QML_SINGLETON
public:
    static DesktopIntegration &instance()
    {
        static DesktopIntegration _instance;
        return _instance;
    }

    static DesktopIntegration *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    Q_INVOKABLE static QString currentDE();
    Q_INVOKABLE static void openShutdownScreen();
    Q_INVOKABLE static void openSystemSettings();
    Q_INVOKABLE static void launchByDesktopId(const QString & desktopId);
    Q_INVOKABLE static QString environmentVariable(const QString & env);
    Q_INVOKABLE static double scaleFactor(const QString & desktopId);
    Q_INVOKABLE static void setScaleFactor(const QString & desktopId, double scaleFactor);
    Q_INVOKABLE static void showFolder(enum QStandardPaths::StandardLocation location);
    Q_INVOKABLE static void showUrl(const QString & url);
    Q_INVOKABLE bool appIsCompulsoryForDesktop(const QString & desktopId);
    // TODO: async get wallpaper?

    Qt::ArrowType dockPosition() const;
    QRect dockGeometry() const;
    uint dockSpacing() const;
    QString backgroundUrl() const;

    Q_INVOKABLE bool isDockedApp(const QString & desktopId) const;
    Q_INVOKABLE void sendToDock(const QString & desktopId);
    Q_INVOKABLE void removeFromDock(const QString & desktopId);
    Q_INVOKABLE bool isOnDesktop(const QString & desktopId) const;
    Q_INVOKABLE void sendToDesktop(const QString & desktopId);
    Q_INVOKABLE void removeFromDesktop(const QString & desktopId);
    Q_INVOKABLE bool isAutoStart(const QString & desktopId) const;
    Q_INVOKABLE void setAutoStart(const QString & desktopId, bool on = true);
    Q_INVOKABLE void uninstallApp(const QString & desktopId);

signals:
    void dockPositionChanged();
    void dockGeometryChanged();
    void dockSpacingChanged();
    void backgroundUrlChanged();

private:
    explicit DesktopIntegration(QObject * parent = nullptr);

    QStringList m_compulsoryAppIdList;
    AppWiz * m_appWizIntegration;
    DdeDock * m_dockIntegration;
    Appearance * m_appearanceIntegration;
};
