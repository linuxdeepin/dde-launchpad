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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <launcherappiconprovider.h>
#include <blurhashimageprovider.h>
#include <launcher1adaptor.h>

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

// we should wait for dtkgui to have a "proper" loadTranslation() to use.
QStringList translationDir() {
    QList<QString> translateDirs;
    auto appName = QCoreApplication::applicationName();
    //("/home/user/.local/share", "/usr/local/share", "/usr/share")
    const QStringList dataDirs(DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / appName / "translations").toString();
    }
#ifdef QT_DEBUG
    translateDirs.prepend(QCoreApplication::applicationDirPath());
#endif
    return translateDirs;
}

// TODO: singleton window
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("dde-launchpad");
    QCoreApplication::setApplicationVersion("0.0.0" + QStringLiteral("-technical-preview"));
    DGuiApplicationHelper::loadTranslator(QStringLiteral("dde-launchpad"), translationDir(), { QLocale() });
    bool isOnlyInstance = DGuiApplicationHelper::setSingleInstance(QStringLiteral("dde-launchpad"));

    if (!isOnlyInstance) {
        qDebug() << "Another instance already exists";
        return 0;
    }

    Launcher1Adaptor adaptor(&LauncherController::instance());
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
        !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
        qWarning() << "register dbus service failed";
    }

    QCommandLineParser parser;
    parser.addOption(LauncherController::instance().optShow);
    parser.addOption(LauncherController::instance().optToggle);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(LauncherController::instance().optShow) || parser.isSet(LauncherController::instance().optToggle)) {
        LauncherController::instance().setVisible(true);
    }

    qmlRegisterUncreatableType<AppItem>("org.deepin.launchpad", 1, 0, "AppItem", "aa");
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "AppsModel", &AppsModel::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "FavoritedProxyModel", &FavoritedProxyModel::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "SearchFilterProxyModel", &SearchFilterProxyModel::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "CategorizedSortProxyModel", &CategorizedSortProxyModel::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "MultipageProxyModel", &MultipageProxyModel::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "DesktopIntegration", &DesktopIntegration::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "LauncherController", &LauncherController::instance());
    qmlRegisterSingletonInstance("org.deepin.launchpad", 1, 0, "DebugHelper", &DebugHelper::instance());

    CategorizedSortProxyModel::instance().setCategoryType(CategorizedSortProxyModel::Alphabetary);

    QQmlApplicationEngine engine;

    QQuickStyle::setStyle("Chameleon");

    engine.addImageProvider(QLatin1String("app-icon"), new LauncherAppIconProvider);
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    QQmlContext * ctx = engine.rootContext();

    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
