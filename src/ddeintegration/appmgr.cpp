// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmgr.h"

#include "AppManager1Application.h"
#include "AppManager1ApplicationObjectManager.h"

#include <DConfig>
#include <DExpected>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDdeIntegration)

DCORE_USE_NAMESPACE

using AppManager1Application = __AppManager1Application;
using AppManager1ApplicationObjectManager = __AppManager1ApplicationObjectManager;
Q_CONSTRUCTOR_FUNCTION(registerComplexDbusType);

static QString parseDisplayName(const QStringMap &source)
{
    static QString key = QLocale::system().name();
    const QString & defaultValue = source.value(u8"default");
    return source.value(key, key.contains('_') ? source.value(key.split('_')[0], defaultValue) : defaultValue);
}

static QString parseName(const QStringMap &source)
{
    return source.value(u8"default");
}

static QString parseIcon(const QStringMap &source)
{
    return source.value(u8"Desktop Entry");
}

template<class T>
static DExpected<T> parseDBusField(const QVariantMap &map, const QString &key)
{
    if (!map.contains(key))
        return {};
    const auto value = map.value(key);
    return DExpected<T>{qdbus_cast<T>(value)};
}

static QString getDisplayName(const bool isDeepin, const QStringMap &name, const QStringMap &genericName)
{
    if (isDeepin) {
        const auto tmp = parseDisplayName(genericName);
        if (!tmp.isEmpty())
            return tmp;
    }

    return parseDisplayName(name);
}

static AppMgr::AppItem *parseDBus2AppItem(const ObjectInterfaceMap &source)
{
    const QVariantMap appInfo = source.value("org.desktopspec.ApplicationManager1.Application");
    if (appInfo.isEmpty())
        return nullptr;

    const auto nodisplay = parseDBusField<bool>(appInfo, u8"NoDisplay");
    if (!nodisplay || nodisplay.value()) {
        return nullptr;
    }
    AppMgr::AppItem *item = nullptr;
    if (auto value = parseDBusField<QString>(appInfo, u8"ID")) {
        item = new AppMgr::AppItem();
        item->id = value.value() + ".desktop";
        item->appId = value.value();
    }

    if (!item) {
        qCWarning(logDdeIntegration) << "Failed to create AppItem";
        return nullptr;
    }

    if (auto value = parseDBusField<QStringList>(appInfo, u8"Categories")) {
        item->categories = value.value();
    }

    // fallback to Name if GenericName is empty, only for X_Deepin_Vendor equals to "deepin".
    const auto deepinVendor = parseDBusField<QString>(appInfo, u8"X_Deepin_Vendor");
    item->vendor = deepinVendor ? deepinVendor.value() : QString();
    
    // Get GenericName field
    const auto genericNameMap = parseDBusField<QStringMap>(appInfo, u8"GenericName");
    item->genericName = genericNameMap ? parseName(genericNameMap.value()) : QString();
    
    item->displayName = getDisplayName(deepinVendor && deepinVendor.value() == QStringLiteral("deepin"),
                                       parseDBusField<QStringMap>(appInfo, u8"Name").value(),
                                       parseDBusField<QStringMap>(appInfo, u8"GenericName").value());

    // just in case the entry is ill-formed, doesn't have a valid display name, fallback to use its desktop-id instead.
    if (item->displayName.isEmpty()) {
        item->displayName = item->id;
    }

    if (auto value = parseDBusField<QStringMap>(appInfo, u8"Name")) {
        item->name = parseName(value.value());
    }

    if (auto value = parseDBusField<QStringMap>(appInfo, u8"Icons")) {
        item->iconName = parseIcon(value.value());
    }

    if (auto value = parseDBusField<qint64>(appInfo, u8"InstalledTime")) {
        item->installedTime = value.value();
    }

    if (auto value = parseDBusField<qint64>(appInfo, u8"LastLaunchedTime")) {
        item->lastLaunchedTime = value.value();
    }

    if (auto value = parseDBusField<bool>(appInfo, u8"AutoStart")) {
        item->isAutoStart = value.value();
    }

    return item;
}

AppMgr::AppMgr(QObject *parent)
    : QObject(parent)
    , m_objectManager(new AppManager1ApplicationObjectManager("org.desktopspec.ApplicationManager1",
                                                              "/org/desktopspec/ApplicationManager1",
                                                              QDBusConnection::sessionBus(), this))
    , m_checkTimer(new QTimer(this))
    , m_checkCount(0)
{
    m_checkTimer->setInterval(3000); // 3 second interval
    connect(m_checkTimer, &QTimer::timeout, this, &AppMgr::checkPendingAppItems);
    initObjectManager();
}

AppMgr::~AppMgr()
{
    for (auto item : std::as_const(m_appItems)) {
        if (auto handler = item->handler) {
            qCDebug(logDdeIntegration) << "Deleting handler for app:" << item->id;
            handler->deleteLater();
        }
    }
    qDeleteAll(m_appItems);
}

AppManager1Application * createAM1AppIfaceByPath(const QString &dbusPath)
{
    AppManager1Application * amAppIface = new AppManager1Application(QLatin1String("org.desktopspec.ApplicationManager1"),
                                                                    dbusPath,
                                                                    QDBusConnection::sessionBus());
    if (!amAppIface->isValid()) {
        qCWarning(logDdeIntegration) << "D-Bus interface not exist or failed to connect to" << dbusPath;
        return nullptr;
    }

    return amAppIface;
}

AppManager1Application * createAM1AppIface(const QString &desktopId)
{
    auto appItem = AppMgr::instance()->appItem(desktopId);
    if (!appItem) {
        qCWarning(logDdeIntegration) << "Can't find appItem for the desktopId" << desktopId;
        return nullptr;
    }
    qCDebug(logDdeIntegration) << "Get app interface for the desktopId" << desktopId;
    return appItem->handler;
}

// if return false, it means the launch is not even started.
// if return true, it means we attempted to launch it via AM, but not sure if it's succeed.
bool AppMgr::launchApp(const QString &desktopId)
{
    qCInfo(logDdeIntegration) << "Launching app:" << desktopId;
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    const auto path = amAppIface->path();
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("dde-am", {"--by-user", path});
    if (!process.waitForFinished()) {
        qCWarning(logDdeIntegration) << "Failed to launch the desktopId:" << desktopId << process.errorString();
        return false;
    } else if (process.exitCode() != 0) {
        qCWarning(logDdeIntegration) << "Failed to launch the desktopId:" << desktopId << process.readAll();
        return false;
    }
    qCInfo(logDdeIntegration) << "Successfully launched desktop ID:" << desktopId;
    return true;
}

bool AppMgr::autoStart(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    return amAppIface->autoStart();
}

void AppMgr::setAutoStart(const QString &desktopId, bool autoStart)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return;
    }

    amAppIface->setAutoStart(autoStart);
    qCDebug(logDdeIntegration) << "Successfully set autoStart for" << desktopId << "to:" << autoStart;
}

static const QStringList DisabledScaleEnvironments {
    "DEEPIN_WINE_SCALE=1",
    "QT_SCALE_FACTOR=1",
    "GDK_SCALE=1",
    "GDK_DPI_SCALE=1",
    "D_DXCB_DISABLE_OVERRIDE_HIDPI=1"
};

bool AppMgr::disableScale(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    const auto environ = amAppIface->environ();
    const QStringList envs(environ.split(';'));
    // return true if envs contains any one of DisabledScaleEnvironments.
    auto iter = std::find_if(envs.begin(), envs.end(), [] (const QString &env) {
        return DisabledScaleEnvironments.contains(env);
    });
    return iter != envs.end();
}

void AppMgr::setDisableScale(const QString &desktopId, bool disableScale)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return;
    }

    QString environ = amAppIface->environ();
    QStringList envs(environ.split(';', Qt::SkipEmptyParts));

    if (disableScale) {
        // remove all ScaleEnvironments, avoid other caller has set it manually.
        envs.removeIf([] (const QString &env) {
            auto iter = std::find_if(DisabledScaleEnvironments.begin(), DisabledScaleEnvironments.end(),
                                     [env] (const QString &item) {
                                         const auto left = item.split('=');
                                         const auto right = env.split('=');
                                         return !right.isEmpty() && left.at(0) == right.at(0);
                                     });
            return iter != DisabledScaleEnvironments.end();
        });
        envs << DisabledScaleEnvironments;
    } else {
        // remove all DisabledScaleEnvironments.
        envs.removeIf([] (const QString &env) {
            return DisabledScaleEnvironments.contains(env);
        });
    }

    environ = envs.join(';');
    qCDebug(logDdeIntegration) << "Update environ for the desktopId" << desktopId << ", env:" << environ;
    amAppIface->setEnviron(environ);
}

bool AppMgr::isOnDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    return amAppIface->isOnDesktop();
}

bool AppMgr::sendToDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    QDBusPendingReply<bool> reply = amAppIface->SendToDesktop();
    reply.waitForFinished();

    if (reply.isError()) {
        qCWarning(logDdeIntegration) << "SendToDesktop failed:" << reply.error();
        return false;
    }

    const bool result = reply.value();
    qCInfo(logDdeIntegration) << "SendToDesktop result for" << desktopId << ":" << result;
    return result;
}

bool AppMgr::removeFromDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) {
        qCWarning(logDdeIntegration) << "Failed to get interface for desktop ID:" << desktopId;
        return false;
    }

    QDBusPendingReply<bool> reply = amAppIface->RemoveFromDesktop();
    reply.waitForFinished();

    if (reply.isError()) {
        qCWarning(logDdeIntegration) << "RemoveFromDesktop failed:" << reply.error();
        return false;
    }

    return reply.value();
}

bool AppMgr::isValid() const
{
    return m_objectManager->isValid();
}

QList<AppMgr::AppItem *> AppMgr::allAppInfosShouldBeShown() const
{
    return m_appItems.values();
}

AppMgr::AppItem *AppMgr::appItem(const QString &id) const
{
    const auto items = m_appItems.values();
    auto iter = std::find_if(items.begin(), items.end(), [id](AppItem *item) {
        return item->id == id;
    });
    return iter != items.end() ? *iter : nullptr;
}

void AppMgr::watchingAppItemPropertyChanged(const QString &key, AppMgr::AppItem *appItem)
{
    AppManager1Application * amAppIface = createAM1AppIfaceByPath(key);
    if (!amAppIface)
        return;

    Q_ASSERT(appItem->handler == nullptr);
    appItem->handler = amAppIface;
    connect(amAppIface, &AppManager1Application::CategoriesChanged, this, [this, appItem](const QStringList & value) {
        qCDebug(logDdeIntegration) << "CategoriesChanged by AM, desktopId" << appItem->id;
        appItem->categories = value;
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::IconsChanged, this, [this, appItem](const QStringMap & value) {
        qCDebug(logDdeIntegration) << "IconsChanged by AM, desktopId" << appItem->id;
        appItem->iconName = parseIcon(value);
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::X_Deepin_VendorChanged, this, [this, appItem, amAppIface](const QString & value) {
        qCDebug(logDdeIntegration) << "X_Deepin_VendorChanged by AM, desktopId" << appItem->id;
        appItem->displayName = getDisplayName(!value.isEmpty(), amAppIface->name(), amAppIface->genericName());
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::GenericNameChanged, this, [this, appItem, amAppIface](const QStringMap & value) {
        qCDebug(logDdeIntegration) << "GenericNameChanged by AM, desktopId" << appItem->id;
        appItem->displayName = getDisplayName(!amAppIface->x_Deepin_Vendor().isEmpty(), amAppIface->name(), value);
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::NameChanged, this, [this, appItem, amAppIface](const QStringMap & value) {
        qCDebug(logDdeIntegration) << "NameChanged by AM, desktopId" << appItem->id;
        appItem->name = parseName(value);
        appItem->displayName = getDisplayName(!amAppIface->x_Deepin_Vendor().isEmpty(), value, amAppIface->genericName());
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::InstalledTimeChanged, this, [this, appItem](const qint64 & value) {
        qCDebug(logDdeIntegration) << "InstalledTimeChanged by AM, desktopId" << appItem->id;
        appItem->installedTime = value;
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::LastLaunchedTimeChanged, this, [this, appItem](const qint64 & value) {
        qCDebug(logDdeIntegration) << "LastLaunchedTimeChanged by AM, desktopId" << appItem->id;
        appItem->lastLaunchedTime = value;
        Q_EMIT itemDataChanged(appItem->id);
    });
    connect(amAppIface, &AppManager1Application::AutoStartChanged, this, [this, appItem](bool value) {
        qCDebug(logDdeIntegration) << "AutoStartChanged by AM, desktopId" << appItem->id;
        appItem->isAutoStart = value;
        Q_EMIT itemDataChanged(appItem->id);
    });
}

void AppMgr::updateAppsLaunchedTimes(const QVariantMap &appsLaunchedTimes)
{
    // need to update times for removed and updated.
    const auto &appItems = m_appItems.values();
    for (const auto item : std::as_const(appItems)) {
        auto iter = appsLaunchedTimes.find(item->appId);
        qint64 times = 0;
        if (iter != appsLaunchedTimes.cend())
            times = iter->toLongLong();

        // including reset and increase times.
        if (item->launchedTimes != times) {
            qCDebug(logDdeIntegration) << "LaunchedTimesChanged by DConfig, desktopId" << item->id << "from" << item->launchedTimes << "to" << times;
            item->launchedTimes = times;
            Q_EMIT itemDataChanged(item->id);
        }
    }
}

void AppMgr::initObjectManager()
{
    if (!isValid()) {
        qCWarning(logDdeIntegration) << "Object manager is not valid, aborting initialization";
        return;
    }

    connect(m_objectManager, &AppManager1ApplicationObjectManager::InterfacesAdded, this,
            [this](const QDBusObjectPath &objPath, ObjectInterfaceMap interfacesAndProperties) {
                const QString key(objPath.path());
                qCDebug(logDdeIntegration) << "InterfacesAdded by AM, path:" << key;
                if (m_appItems.contains(objPath.path())) {
                    qWarning() << "App already exists for the path:" << key;
                    return;
                }
                // Reset check count when new app is added
                m_checkCount = 0;
                if (auto appItem = parseDBus2AppItem(interfacesAndProperties)) {
                    qCDebug(logDdeIntegration) << "App item added, desktopId" << appItem->id;
                    watchingAppItemAdded(key, appItem);
                }
            });
    connect(m_objectManager, &AppManager1ApplicationObjectManager::InterfacesRemoved, this,
            [this](const QDBusObjectPath &objPath, const QStringList &interfaces) {
                Q_UNUSED(interfaces)
                const QString key(objPath.path());
                qCDebug(logDdeIntegration) << "InterfacesRemoved by AM, path:" << key;
                watchingAppItemRemoved(key);
            });

    fetchAppItems();

    DConfig *config = DConfig::create("org.deepin.dde.application-manager", "org.deepin.dde.am", "", this);
    if (!config->isValid()) {
        qCWarning(logDdeIntegration) << "DConfig is invalid when getting launched times.";
    } else {
        static const QString AppsLaunchedTimes(u8"appsLaunchedTimes");
        const auto &value = config->value(AppsLaunchedTimes).toMap();
        updateAppsLaunchedTimes(value);
        QObject::connect(config, &DConfig::valueChanged, this, [this, config](const QString &key) {
            if (key != AppsLaunchedTimes) {
                qCDebug(logDdeIntegration) << "Ignoring non-appsLaunchedTimes key:" << key;
                return;
            }

            qCInfo(logDdeIntegration) << "appsLaunchedTimes of DConfig changed, updating";
            const auto &value = config->value(AppsLaunchedTimes).toMap();
            updateAppsLaunchedTimes(value);
        });
    }
}

void AppMgr::fetchAppItems()
{
    qCDebug(logDdeIntegration) << "Begin to fetch apps.";
    const auto reply = m_objectManager->GetManagedObjects();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call){
        QDBusPendingReply<ObjectMap> reply = *call;
        if (reply.isError()) {
            qWarning() << "Failed to get apps from AM, " << reply.error();
            call->deleteLater();
            return;
        }
        qCDebug(logDdeIntegration) << "Fetched all AppItem, and start parsing data.";
        QMap<QString, AppMgr::AppItem *> items;
        const auto objects = reply.value();
        for (auto iter = objects.cbegin(); iter != objects.cend(); ++iter) {
            const auto &objPath = iter.key();
            const ObjectInterfaceMap &objs = iter.value();
            auto appItem = parseDBus2AppItem(objs);
            if (!appItem) {
                continue;
            }

            items[objPath.path()] = appItem;
            watchingAppItemPropertyChanged(objPath.path(), appItem);
        }
        call->deleteLater();
        qCDebug(logDdeIntegration) << "Fetched all AppItem, and end up parsing data.";

        m_appItems = items;
        Q_EMIT changed();
    });
    // TODO async to fetch apps.
    watcher->waitForFinished();
}

void AppMgr::watchingAppItemAdded(const QString &key, AppItem *appItem)
{
    // Check if iconName is an absolute path and if the file exists
    if (isAbsolutePathIcon(appItem->iconName)) {
        QFileInfo fileInfo(appItem->iconName);
        if (!fileInfo.exists()) {
            // File doesn't exist, add to pending container
            m_pendingAppItems[key] = appItem;
            
            // Start timer if not already running
            if (!m_checkTimer->isActive()) {
                m_checkTimer->start();
            }
            return;
        }
    }
    
    // Icon exists or is a system icon, proceed with normal logic
    m_appItems[key] = appItem;
    watchingAppItemPropertyChanged(key, appItem);
    Q_EMIT changed();
}

void AppMgr::watchingAppItemRemoved(const QString &key)
{
    // Check if the item is in pending container
    if (m_pendingAppItems.contains(key)) {
        auto pendingAppItem = m_pendingAppItems.value(key);
        qCDebug(logDdeIntegration) << "Removing pending app item, desktopId" << pendingAppItem->id;
        m_pendingAppItems.remove(key);
        delete pendingAppItem;
        return;
    }
    
    auto appItem = m_appItems.value(key);
    if (!appItem) {
        qCWarning(logDdeIntegration) << "App item not found for key:" << key;
        return;
    }

    qCDebug(logDdeIntegration) << "App item removed, desktopId" << appItem->id;
    if (auto handler = appItem->handler) {
        qCDebug(logDdeIntegration) << "Deleting handler for removed app:" << appItem->id;
        handler->disconnect(this);
        handler->deleteLater();
    }
    m_appItems.remove(key);
    delete appItem;
    Q_EMIT changed();
}

void AppMgr::checkPendingAppItems()
{
    m_checkCount++;
    if (m_pendingAppItems.isEmpty()) {
        m_checkTimer->stop();
        return;
    }
    
    QList<QPair<QString, AppItem *>> itemsToProcess;
    
    // Check all pending items
    for (auto it = m_pendingAppItems.begin(); it != m_pendingAppItems.end(); ) {
        const QString &key = it.key();
        AppItem *appItem = it.value();
        
        if (isAbsolutePathIcon(appItem->iconName)) {
            QFileInfo fileInfo(appItem->iconName);
            if (fileInfo.exists()) {
                // File now exists, add to main container
                itemsToProcess.append(qMakePair(key, appItem));
                it = m_pendingAppItems.erase(it);
                continue;
            }
        }
        ++it;
    }
    
    // Process items whose icons now exist
    for (const auto &itemPair : itemsToProcess) {
        const QString &key = itemPair.first;
        AppItem *appItem = itemPair.second;
        
        m_appItems[key] = appItem;
        watchingAppItemPropertyChanged(key, appItem);
        Q_EMIT changed();
    }
    
    // Check if timeout reached (60 seconds)
    if (m_checkCount >= 20 && !m_pendingAppItems.isEmpty()) {        
        // Force process all remaining pending items
        for (auto it = m_pendingAppItems.begin(); it != m_pendingAppItems.end(); ) {
            const QString &key = it.key();
            AppItem *appItem = it.value();
            
            m_appItems[key] = appItem;
            watchingAppItemPropertyChanged(key, appItem);
            Q_EMIT changed();
            
            it = m_pendingAppItems.erase(it);
        }
        
        m_checkTimer->stop();
    } else if (m_pendingAppItems.isEmpty()) {
        m_checkTimer->stop();
    }
}

bool AppMgr::isAbsolutePathIcon(const QString &iconName) const
{
    // Check if the icon name is an absolute path (starts with /)
    return iconName.startsWith('/');
}

AppMgr *AppMgr::instance() {
    static AppMgr gInstance;
    return &gInstance;
}
