// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopintegration.h"

#include <DDBusSender>
#include <DDesktopEntry>
#include <DStandardPaths>
#include <QRect>
#include <appinfo.h>
#include <appmgr.h>

#include <AppStreamQt/pool.h>

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

bool DesktopIntegration::appIsCompulsoryForDesktop(const QString &desktopId)
{
    const QString currentDE(DesktopIntegration::currentDE());

    AppStream::Pool pool;
    // qDebug() << pool.flags() << currentDE;
    pool.load();

    const QList<AppStream::Component> components = pool.componentsByLaunchable(AppStream::Launchable::KindDesktopId, desktopId);
    for (const AppStream::Component & component : components) {
        return component.compulsoryForDesktops().contains(currentDE);
    }
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
    QString desktopItemPath = desktopItemFilePath(desktopId);
    if (desktopItemPath.isEmpty()) return;
    if (QFile::exists(desktopItemPath)) return;

    QString srcFilePath = AppInfo::fullPathByDesktopId(desktopId);
    if (srcFilePath.isEmpty()) return;

    bool copied = QFile::copy(srcFilePath, desktopItemPath);
    if (!copied) return;
    DDesktopEntry entry(desktopItemPath);
    entry.setStringValue("DDE", "X-Deepin-CreatedBy"); // maybe better to add a "managed by"?
    // There was originally also a "X-Deepin-AppID" which... doesn't seems to make any sense
    if (entry.save()) {
        // TODO: play sfx for this
        // DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_SendFileComplete); // why is this in dtkwidget?
    }
}

void DesktopIntegration::removeFromDesktop(const QString &desktopId)
{
    QString desktopItemPath = desktopItemFilePath(desktopId);
    if (desktopItemPath.isEmpty()) return;

    QFile desktopItemFile(desktopItemPath);

    if (desktopItemFile.exists()) {
        desktopItemFile.remove();
    }
}

// TODO: should also consider OnlyShownIn and NotShownIn
bool DesktopIntegration::isAutoStart(const QString &desktopId) const
{
    const QStringList xdgConfig = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    const QString autoStartFileRelPath(QString("autostart/%1").arg(desktopId));
    for (const QString & path : xdgConfig) {
        const QString & autoStartPath(QDir(path).absoluteFilePath(autoStartFileRelPath));
        if (QFile::exists(autoStartPath)) {
            DDesktopEntry entry(autoStartPath);
            if (entry.rawValue("Hidden", "Desktop Entry", "false") == QLatin1String("false")) {
                return true;
            }
            return false;
        }
    }
    return false;
}

// only affact the one in XDG_CONFIG_HOME, don't care about the system one (even if there is one).
void DesktopIntegration::setAutoStart(const QString &desktopId, bool on)
{
    QString srcFilePath = AppInfo::fullPathByDesktopId(desktopId);
    if (srcFilePath.isEmpty()) return;

    const QString autoStartFileRelPath(QString("autostart/%1").arg(desktopId));
    const QString autoStartPath(QDir(DStandardPaths::path(DStandardPaths::XDG::ConfigHome)).absoluteFilePath(autoStartFileRelPath));

    // Ensure there is a autostart entry file under the $XDG_CONFIG_HOME/autostart/ folder
    // Ee always create this file since the *system* might *have* one entry with Hidden=true,
    // which need us to override (even though it's very not likely to happen).
    bool createdByUs = false;
    if (!QFile::exists(autoStartPath)) {
        QString srcFilePath = AppInfo::fullPathByDesktopId(desktopId);
        bool succ = QFile::copy(srcFilePath, autoStartPath);
        if (!succ) {
            return;
        }
        createdByUs = true;
    }

    DDesktopEntry entry(autoStartPath);
    entry.setRawValue(on ? "false" : "true", "Hidden");
    if (createdByUs) {
        entry.setStringValue("DDE", "X-Deepin-CreatedBy"); // maybe better to add a "managed by"?
    }
    entry.save();
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
    connect(m_dockIntegration, &DdeDock::directionChanged, this, &DesktopIntegration::dockPositionChanged);
    connect(m_dockIntegration, &DdeDock::geometryChanged, this, &DesktopIntegration::dockGeometryChanged);
    connect(m_appearanceIntegration, &Appearance::wallpaperBlurhashChanged, this, &DesktopIntegration::backgroundUrlChanged);
}
