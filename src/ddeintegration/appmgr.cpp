// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmgr.h"

#include "AppManager1Application.h"
#include "AppManager1ApplicationObjectManager.h"

#include <DExpected>
DCORE_USE_NAMESPACE

using AppManager1Application = __AppManager1Application;
using AppManager1ApplicationObjectManager = __AppManager1ApplicationObjectManager;
Q_CONSTRUCTOR_FUNCTION(registerComplexDbusType);
static const qulonglong InvalidTime = std::numeric_limits<qint64>().max();

static QString parseDisplayName(const QStringMap &source)
{
    static QString key = QLocale::system().name();
    return source.value(key, source.value(u8"default"));
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
    }

    if (!item) {
        return nullptr;
    }

    if (auto value = parseDBusField<QStringList>(appInfo, u8"Categories")) {
        item->categories = value.value();
    }

    if (auto value = parseDBusField<QStringMap>(appInfo, u8"Name")) {
        item->displayName = parseDisplayName(value.value());
    }

    if (auto value = parseDBusField<QStringMap>(appInfo, u8"Icons")) {
        item->iconName = parseIcon(value.value());
    }

    if (auto value = parseDBusField<qulonglong>(appInfo, u8"installedTime")) {
        item->installedTime = value.value();
        if (item->installedTime >= InvalidTime) {
            qWarning() << "Invalid installedTime for the desktopId" << item->id;
            item->installedTime = 0;
        }
    }

    if (auto value = parseDBusField<qulonglong>(appInfo, u8"LastLaunchedTime")) {
        item->lastLaunchedTime = value.value();
        if (item->lastLaunchedTime >= InvalidTime) {
            qWarning() << "Invalid LastLaunchedTime for the desktopId" << item->id;
            item->lastLaunchedTime = 0;
        }
    }

    return item;
}

AppMgr::AppMgr(QObject *parent)
    : QObject(parent)
    , m_objectManager(new AppManager1ApplicationObjectManager("org.desktopspec.ApplicationManager1",
                                                              "/org/desktopspec/ApplicationManager1",
                                                              QDBusConnection::sessionBus(), this))
{
    initObjectManager();
}

AppMgr::~AppMgr()
{
    for (auto item : std::as_const(m_appItems)) {
        if (auto handler = item->handler) {
            handler->deleteLater();
        }
    }
    qDeleteAll(m_appItems);
}

inline QString escapeToObjectPath(const QString &str)
{
    if (str.isEmpty()) {
        return "_";
    }

    auto ret = str;
    static QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(_%1)"}.arg(hexStr));
        }
    }
    return ret;
}

AppManager1Application * createAM1AppIfaceByPath(const QString &dbusPath)
{
    AppManager1Application * amAppIface = new AppManager1Application(QLatin1String("org.desktopspec.ApplicationManager1"),
                                                                    dbusPath,
                                                                    QDBusConnection::sessionBus());
    if (!amAppIface->isValid()) {
        qDebug() << "D-Bus interface not exist or failed to connect to" << dbusPath;
        return nullptr;
    }

    return amAppIface;
}

AppManager1Application * createAM1AppIface(const QString &desktopId)
{
    // the new dde-application-manager use systemd-style Application ID, which is
    // basicly the freedesktop desktop-id sins the ".desktop" suffix.
    constexpr int suffixLen = std::char_traits<char>::length(".desktop");
    QString systemdAppId(desktopId.chopped(suffixLen));
    QString dbusPath = QString("/org/desktopspec/ApplicationManager1/%1").arg(escapeToObjectPath(systemdAppId));

    return createAM1AppIfaceByPath(dbusPath);
}

// if return false, it means the launch is not even started.
// if return true, it means we attempted to launch it via AM, but not sure if it's succeed.
bool AppMgr::launchApp(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return false;

    amAppIface->Launch(QString(), QStringList{}, QVariantMap());

    return true;
}

bool AppMgr::autoStart(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return false;

    return amAppIface->autoStart();
}

void AppMgr::setAutoStart(const QString &desktopId, bool autoStart)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return;

    amAppIface->setAutoStart(autoStart);
}

// 0: global scaleFactor
double AppMgr::scaleFactor(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return 0;

    return amAppIface->scaleFactor();
}

void AppMgr::setScaleFactor(const QString &desktopId, double scaleFactor)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return;

    amAppIface->setScaleFactor(scaleFactor);
}

bool AppMgr::isOnDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return false;

    return amAppIface->isOnDesktop();
}

bool AppMgr::sendToDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return false;

    QDBusPendingReply<bool> reply = amAppIface->SendToDesktop();
    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error();
        return false;
    }

    return reply.value();
}

bool AppMgr::removeFromDesktop(const QString &desktopId)
{
    AppManager1Application * amAppIface = createAM1AppIface(desktopId);
    if (!amAppIface) return false;

    QDBusPendingReply<bool> reply = amAppIface->RemoveFromDesktop();
    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error();
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

void AppMgr::watchingAppItemPropertyChanged(const QString &key, AppMgr::AppItem *appItem)
{
    AppManager1Application * amAppIface = createAM1AppIfaceByPath(key);
    if (!amAppIface)
        return;

    Q_ASSERT(appItem->handler == nullptr);
    appItem->handler = amAppIface;
    connect(amAppIface, &AppManager1Application::CategoriesChanged, this, [this, appItem](const QStringList & value) {
        qDebug() << "CategoriesChanged by AM, desktopId" << appItem->id;
        appItem->categories = value;
        Q_EMIT changed();
    });
    connect(amAppIface, &AppManager1Application::IconsChanged, this, [this, appItem](const QStringMap & value) {
        qDebug() << "IconsChanged by AM, desktopId" << appItem->id;
        appItem->iconName = parseIcon(value);
        Q_EMIT changed();
    });
    connect(amAppIface, &AppManager1Application::NameChanged, this, [this, appItem](const QStringMap & value) {
        qDebug() << "NameChanged by AM, desktopId" << appItem->id;
        appItem->displayName = parseDisplayName(value);
        Q_EMIT changed();
    });
    connect(amAppIface, &AppManager1Application::InstalledTimeChanged, this, [this, appItem](const qulonglong & value) {
        qDebug() << "InstalledTimeChanged by AM, desktopId" << appItem->id;
        appItem->installedTime = value;
        Q_EMIT changed();
    });
    connect(amAppIface, &AppManager1Application::LastLaunchedTimeChanged, this, [this, appItem](const qulonglong & value) {
        qDebug() << "LastLaunchedTimeChanged by AM, desktopId" << appItem->id;
        appItem->lastLaunchedTime = value;
        Q_EMIT changed();
    });
}

void AppMgr::initObjectManager()
{
    if (!isValid())
        return;

    connect(m_objectManager, &AppManager1ApplicationObjectManager::InterfacesAdded, this,
            [this](const QDBusObjectPath &objPath, ObjectInterfaceMap interfacesAndProperties) {
                const QString key(objPath.path());
                qDebug() << "InterfacesAdded by AM, path:" << key;
                if (m_appItems.contains(objPath.path())) {
                    qWarning() << "App already exists for the path:" << key;
                    return;
                }
                if (auto appItem = parseDBus2AppItem(interfacesAndProperties)) {
                    qDebug() << "App item added, desktopId" << appItem->id;
                    watchingAppItemAdded(key, appItem);
                }
            });
    connect(m_objectManager, &AppManager1ApplicationObjectManager::InterfacesRemoved, this,
            [this](const QDBusObjectPath &objPath, const QStringList &interfaces) {
                const QString key(objPath.path());
                qDebug() << "InterfacesRemoved by AM, path:" << key;
                watchingAppItemRemoved(key);
            });

    fetchAppItems();
}

void AppMgr::fetchAppItems()
{
    qDebug() << "Begin to fetch apps.";
    const auto reply = m_objectManager->GetManagedObjects();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call){
        QDBusPendingReply<ObjectMap> reply = *call;
        if (reply.isError()) {
            qWarning() << "Failed to get apps from AM, " << reply.error();
            call->deleteLater();
            return;
        }
        qDebug() << "Fetched all AppItem, and start parsing data.";
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
        qDebug() << "Fetched all AppItem, and end up parsing data.";

        m_appItems = items;
        Q_EMIT changed();
    });
    // TODO async to fetch apps.
    watcher->waitForFinished();
}

void AppMgr::watchingAppItemAdded(const QString &key, AppItem *appItem)
{
    m_appItems[key] = appItem;
    watchingAppItemPropertyChanged(key, appItem);
    Q_EMIT changed();
}

void AppMgr::watchingAppItemRemoved(const QString &key)
{
    auto appItem = m_appItems.value(key);
    if (!appItem)
        return;

    qDebug() << "App item removed, desktopId" << appItem->id;
    if (auto handler = appItem->handler) {
        handler->deleteLater();
    }
    m_appItems.remove(key);
    delete appItem;
    Q_EMIT changed();
}

AppMgr *AppMgr::instance() {
    static AppMgr gInstance;
    return &gInstance;
}
