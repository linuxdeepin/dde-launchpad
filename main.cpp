// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "categorizedsortproxymodel.h"
#include "launchercontroller.h"
#include "personalizationmanager.h"

#include <QDBusConnection>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QWindow>
#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <launcherappiconprovider.h>
#include <launcherfoldericonprovider.h>
#include <blurhashimageprovider.h>
#include <DLog>

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

int main(int argc, char* argv[])
{
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::DontSaveApplicationTheme, true);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);

    QGuiApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("dde-launchpad");
    QCoreApplication::setApplicationVersion(QT_STRINGIFY(DDE_LAUNCHPAD_VERSION) + QStringLiteral("-technical-preview"));
    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();
    Dtk::Core::DLogManager::registerJournalAppender();
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

    QQmlApplicationEngine engine;

    QQuickStyle::setStyle("Chameleon");

    engine.addImageProvider(QLatin1String("app-icon"), new LauncherAppIconProvider);
    engine.addImageProvider(QLatin1String("folder-icon"), new LauncherFolderIconProvider);
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    QQmlContext * ctx = engine.rootContext();

    engine.loadFromModule("org.deepin.launchpad", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;
    QWindow * windowdFrameWindow = engine.rootObjects().at(0)->findChild<QWindow *>("WindowedFrameApplicationWindow");
    Q_CHECK_PTR(windowdFrameWindow);
    PersonalizationManager personalizationmgr;
    if (windowdFrameWindow) {
        personalizationmgr.personalizeWindow(windowdFrameWindow, PersonalizationManager::BgBlurredWallpaper);
    }

    return app.exec();
}
