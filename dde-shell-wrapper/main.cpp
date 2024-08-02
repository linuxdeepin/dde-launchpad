// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "categorizedsortproxymodel.h"
#include "launchercontroller.h"
#include "personalizationmanager.h"

#include <QDBusConnection>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <launcherappiconprovider.h>
#include <launcherfoldericonprovider.h>
#include <blurhashimageprovider.h>

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
    explicit LanchpadPanel(QObject *parent)
        : DPanel(parent)
    {
    }
    virtual bool load() override
    {
        DGuiApplicationHelper::loadTranslator(QStringLiteral("dde-launchpad"), translationDir(), { QLocale::system() });

        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.registerService(QStringLiteral("org.deepin.dde.Launcher1")) ||
            !connection.registerObject(QStringLiteral("/org/deepin/dde/Launcher1"), &LauncherController::instance())) {
            qWarning() << "register dbus service failed";
            return false;
        }

        auto &engine = *DQmlEngine().engine();

        engine.addImageProvider(QLatin1String("app-icon"), new LauncherAppIconProvider);
        engine.addImageProvider(QLatin1String("folder-icon"), new LauncherFolderIconProvider);
        engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

        connect(engine, &DQmlEngine::finished, this, [this](){
            QWindow * windowdFrameWindow = engine.rootObject()->findChild<QWindow *>("WindowedFrameApplicationWindow");
            Q_CHECK_PTR(windowdFrameWindow);
            PersonalizationManager * personalizationmgr = new PersonalizationManager(this);
            if (windowdFrameWindow) {
                personalizationmgr->personalizeWindow(windowdFrameWindow, PersonalizationManager::BgBlurredWallpaper);
            }
        });

        return DPanel::load();
    }
};

D_APPLET_CLASS(LanchpadPanel)

#include "main.moc"
