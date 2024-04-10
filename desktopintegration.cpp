// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopintegration.h"

#include <DConfig>
#include <DDBusSender>
#include <DDesktopEntry>
#include <DStandardPaths>
#include <DDesktopServices>
#include <QRect>
#include <appinfo.h>
#include <appmgr.h>

#ifndef NO_APPSTREAM_QT
#include <AppStreamQt/pool.h>
#endif

#include "appwiz.h"
#include "ddedock.h"
#include "appearance.h"

DCORE_USE_NAMESPACE

QString DesktopIntegration::currentDE()
{
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP", QStringLiteral("DDE")).split(':').constFirst();
}

void DesktopIntegration::openShutdownScreen()
{
    DDBusSender()
        .service("org.deepin.dde.ShutdownFront1")
        .interface("org.deepin.dde.ShutdownFront1")
        .path("/org/deepin/dde/ShutdownFront1")
        .method("Show")
        .call();
}

void DesktopIntegration::openSystemSettings()
{
    DDBusSender()
        .service("org.deepin.dde.ControlCenter1")
        .interface("org.deepin.dde.ControlCenter1")
        .path("/org/deepin/dde/ControlCenter1")
        .method(QString("Show"))
        .call();
}

void DesktopIntegration::launchByDesktopId(const QString &desktopId)
{
    if (!AppMgr::launchApp(desktopId)) {
        AppInfo::launchByDesktopId(desktopId);
    }
}

QString DesktopIntegration::environmentVariable(const QString &env)
{
    return qEnvironmentVariable(env.toStdString().c_str());
}

double DesktopIntegration::disableScale(const QString &desktopId)
{
    return AppMgr::disableScale(desktopId);
}

void DesktopIntegration::setDisableScale(const QString &desktopId, double disableScale)
{
    return AppMgr::setDisableScale(desktopId, disableScale);
}

void DesktopIntegration::showFolder(QStandardPaths::StandardLocation location)
{
    QStringList paths(QStandardPaths::standardLocations(location));
    if (!paths.isEmpty()) {
        Dtk::Gui::DDesktopServices::showFolder(paths.constFirst());
    }
}

void DesktopIntegration::showUrl(const QString &url)
{
    Dtk::Gui::DDesktopServices::showFolder(QUrl(url));
}

bool DesktopIntegration::appIsCompulsoryForDesktop(const QString &desktopId)
{
    if (m_compulsoryAppIdList.contains(desktopId)) return true;

#ifdef NO_APPSTREAM_QT
    Q_UNUSED(desktopId)
#else
    const QString currentDE(DesktopIntegration::currentDE());

    AppStream::Pool pool;
    // qDebug() << pool.flags() << currentDE;
    pool.load();

    const AppStream::ComponentBox components = pool.componentsByLaunchable(AppStream::Launchable::KindDesktopId, desktopId);
    for (const AppStream::Component & component : components) {
        return component.compulsoryForDesktops().contains(currentDE);
    }
#endif

    return false;
}

Qt::ArrowType DesktopIntegration::dockPosition() const
{
    return m_dockIntegration->direction();
}

// If position (x, y) is unknown, it's okay to return -1 as position value.
QRect DesktopIntegration::dockGeometry() const
{
    return m_dockIntegration->geometry();
}

uint DesktopIntegration::dockSpacing() const
{
    return 10;
}

QString DesktopIntegration::backgroundUrl() const
{
    return QString("image://blurhash/%1").arg(m_appearanceIntegration->wallpaperBlurhash());
}

bool DesktopIntegration::isDockedApp(const QString &desktopId) const
{
    // This is something we shouldn't do but anyway...
    const QString & fullPath = AppInfo::fullPathByDesktopId(desktopId);
    // Seems QML's list type doesn't have a contains() method...
    return m_dockIntegration->isDocked(fullPath);
}

void DesktopIntegration::sendToDock(const QString &desktopId)
{
    const QString & fullPath = AppInfo::fullPathByDesktopId(desktopId);
    return m_dockIntegration->sendToDock(fullPath);
}

void DesktopIntegration::removeFromDock(const QString &desktopId)
{
    const QString & fullPath = AppInfo::fullPathByDesktopId(desktopId);
    return m_dockIntegration->removeFromDock(fullPath);
}

inline QString desktopItemFilePath(const QString &desktopId)
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty()) return QString();

    QDir desktopDir(desktopPath);
    return desktopDir.filePath(desktopId);
}

bool DesktopIntegration::isOnDesktop(const QString &desktopId) const
{
    QString desktopItemPath = desktopItemFilePath(desktopId);
    if (desktopItemPath.isEmpty()) return false;
    return QFileInfo::exists(desktopItemPath);
}

void DesktopIntegration::sendToDesktop(const QString &desktopId)
{
    if (AppMgr::sendToDesktop(desktopId)) {
        Dtk::Gui::DDesktopServices::playSystemSoundEffect(Dtk::Gui::DDesktopServices::SSE_SendFileComplete);
    }
}

void DesktopIntegration::removeFromDesktop(const QString &desktopId)
{
    AppMgr::removeFromDesktop(desktopId);
}

bool DesktopIntegration::isAutoStart(const QString &desktopId) const
{
    return AppMgr::autoStart(desktopId);
}

// only affect the one in XDG_CONFIG_HOME, don't care about the system one (even if there is one).
void DesktopIntegration::setAutoStart(const QString &desktopId, bool on)
{
    return AppMgr::setAutoStart(desktopId, on);
}

void DesktopIntegration::uninstallApp(const QString &desktopId)
{
    const QString & fullPath = AppInfo::fullPathByDesktopId(desktopId);
    m_appWizIntegration->legacyRequestUninstall(fullPath);
}

DesktopIntegration::DesktopIntegration(QObject *parent)
    : QObject(parent)
    , m_appWizIntegration(new AppWiz(this))
    , m_dockIntegration(new DdeDock(this))
    , m_appearanceIntegration(new Appearance(this))
{
    QScopedPointer<DConfig> dconfig(DConfig::create("dde-launchpad", "org.deepin.dde.launchpad.appsmodel"));
    Q_ASSERT_X(dconfig->isValid(), "DConfig", "DConfig file is missing or invalid");
    // TODO:
    //   1. ensure dde-launchpad is build with AppStream support
    //   2. ensure dde-control-center, deepin-calendar, dde-file-manager ship their AppStream MetaInfo file
    //   3. remove the hard-coded list below
    static const QStringList defaultCompulsoryAppIdList{
        "dde-control-center.desktop",
        "dde-computer.desktop",
        "dde-trash.desktop",
        "dde-file-manager.desktop",
        "deepin-terminal.desktop",
        "deepin-manual.desktop",
        "deepin-system-monitor.desktop",
        "deepin-devicemanager.desktop",
        "dde-printer.desktop",
        "deepin-app-store.desktop",
        "dde-calendar.desktop"
    };
    m_compulsoryAppIdList = dconfig->value("compulsoryAppIdList", defaultCompulsoryAppIdList).toStringList();

    connect(m_dockIntegration, &DdeDock::directionChanged, this, &DesktopIntegration::dockPositionChanged);
    connect(m_dockIntegration, &DdeDock::geometryChanged, this, &DesktopIntegration::dockGeometryChanged);
    connect(m_appearanceIntegration, &Appearance::wallpaperBlurhashChanged, this, &DesktopIntegration::backgroundUrlChanged);
    connect(m_appearanceIntegration, &Appearance::opacityChanged, this, &DesktopIntegration::opacityChanged);
}

qreal DesktopIntegration::opacity() const
{
    return m_appearanceIntegration->opacity();
}
