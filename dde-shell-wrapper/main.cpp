// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "desktopintegration.h"
#include "favoritedproxymodel.h"
#include "searchfilterproxymodel.h"
#include "categorizedsortproxymodel.h"
#include "multipageproxymodel.h"
#include "launchercontroller.h"
#include "debughelper.h"

#include <QDBusConnection>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <launcherappiconprovider.h>
#include <launcherfoldericonprovider.h>
#include <blurhashimageprovider.h>
#include <multipagesortfilterproxymodel.h>

#include "panel.h"
#include "qmlengine.h"
#include "pluginfactory.h"
DS_USE_NAMESPACE

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

// we should wait for dtkgui to have a "proper" loadTranslation() to use.
QStringList translationDir() {
    QList<QString> translateDirs;
    QString appName{"dde-launchpad"};
    //("/home/user/.local/share", "/usr/local/share", "/usr/share")
    const QStringList dataDirs(DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / appName / "translations").toString();
    }
    return translateDirs;
}

class LanchpadPanel : public DPanel
{
    Q_OBJECT
public:
    LanchpadPanel(QObject *parent)
        : DPanel(parent)
    {
    }
    virtual bool load() override
    {
        DGuiApplicationHelper::loadTranslator(QStringLiteral("dde-launchpad"), translationDir(), { QLocale() });

        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
            !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
            qWarning() << "register dbus service failed";
        }

        qmlRegisterType<MultipageSortFilterProxyModel>("org.deepin.launchpad", 1, 0, "MultipageSortFilterProxyModel");
        qmlRegisterUncreatableType<AppItem>("org.deepin.launchpad", 1, 0, "AppItem", "AppItem should only be created from C++ side");
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "AppsModel", &AppsModel::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "FavoritedProxyModel", &FavoritedProxyModel::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "SearchFilterProxyModel", &SearchFilterProxyModel::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "CategorizedSortProxyModel", &CategorizedSortProxyModel::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "MultipageProxyModel", &MultipageProxyModel::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "DesktopIntegration", &DesktopIntegration::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "LauncherController", &LauncherController::instance());
        qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "DebugHelper", &DebugHelper::instance());

        CategorizedSortProxyModel::instance().setCategoryType(CategorizedSortProxyModel::Alphabetary);

        auto &engine = *DQmlEngine().engine();

        engine.addImageProvider(QLatin1String("app-icon"), new LauncherAppIconProvider);
        engine.addImageProvider(QLatin1String("folder-icon"), new LauncherFolderIconProvider);
        engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

        return DPanel::load();
    }
    virtual bool init() override
    {
        return DPanel::init();
    }
};

D_APPLET_CLASS(LanchpadPanel)

#include "main.moc"
