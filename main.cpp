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

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

int main(int argc, char* argv[])
{
    // workaround for https://github.com/linuxdeepin/dtk/issues/115
    qputenv("D_POPUP_MODE", "embed");

    QGuiApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("dde-launchpad");
    QCoreApplication::setApplicationVersion(QT_STRINGIFY(DDE_LAUNCHPAD_VERSION) + QStringLiteral("-technical-preview"));
    DGuiApplicationHelper::loadTranslator();
    bool isOnlyInstance = DGuiApplicationHelper::setSingleInstance(QStringLiteral("dde-launchpad"));

    QCommandLineParser parser;
    parser.addOption(LauncherController::instance().optShow);
    parser.addOption(LauncherController::instance().optToggle);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    if (!isOnlyInstance) {
        qDebug() << "Another instance already exists";
        return 0;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
        !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
        qWarning() << "register dbus service failed";
    }

    if (parser.isSet(LauncherController::instance().optShow) || parser.isSet(LauncherController::instance().optToggle)) {
        LauncherController::instance().setVisible(true);
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

    QQmlApplicationEngine engine;

    QQuickStyle::setStyle("Chameleon");

    engine.addImageProvider(QLatin1String("app-icon"), new LauncherAppIconProvider);
    engine.addImageProvider(QLatin1String("folder-icon"), new LauncherFolderIconProvider);
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    QQmlContext * ctx = engine.rootContext();

    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
