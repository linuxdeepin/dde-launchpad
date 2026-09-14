// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmgr.h"

#include <DUtil>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QProcess>
#include <QTimer>

#include <algorithm>

Q_LOGGING_CATEGORY(logAppMgr, "org.deepin.dde.launchpad.appmgr")

namespace
{
const QString ApplicationManagerService = QStringLiteral("org.desktopspec.ApplicationManager1");
const QString ApplicationInterface = QStringLiteral("org.desktopspec.ApplicationManager1.Application");
const QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString ApplicationObjectPathPrefix = QStringLiteral("/org/desktopspec/ApplicationManager1/");

const QStringList &disabledScaleEnvironments()
{
    static const QStringList environments {
        QStringLiteral("DEEPIN_WINE_SCALE=1"),
        QStringLiteral("QT_SCALE_FACTOR=1"),
        QStringLiteral("GDK_SCALE=1"),
        QStringLiteral("GDK_DPI_SCALE=1"),
        QStringLiteral("D_DXCB_DISABLE_OVERRIDE_HIDPI=1"),
    };
    return environments;
}

QString applicationObjectPath(QString desktopId)
{
    if (desktopId.endsWith(QLatin1String(".desktop")))
        desktopId.chop(8);
    if (desktopId.isEmpty() || desktopId.startsWith(QLatin1String("internal/")))
        return {};
    return ApplicationObjectPathPrefix + DUtil::escapeToObjectPath(desktopId);
}

bool isScaleDisabled(const QString &environment)
{
    const QStringList values = environment.split(';', Qt::SkipEmptyParts);
    return std::any_of(values.cbegin(), values.cend(), [](const QString &value) {
        return disabledScaleEnvironments().contains(value);
    });
}

QString scaleEnvironment(const QString &environment, bool disabled)
{
    QStringList values = environment.split(';', Qt::SkipEmptyParts);
    if (disabled) {
        values.removeIf([](const QString &value) {
            const QString variable = value.section('=', 0, 0);
            return std::any_of(disabledScaleEnvironments().cbegin(),
                               disabledScaleEnvironments().cend(),
                               [&variable](const QString &override) {
                return override.section('=', 0, 0) == variable;
            });
        });
        values.append(disabledScaleEnvironments());
    } else {
        for (const QString &value : disabledScaleEnvironments())
            values.removeAll(value);
    }
    return values.join(';');
}
}

AppMgr::AppMgr(QObject *parent)
    : QObject(parent)
    , m_checkTimer(new QTimer(this))
{
    m_checkTimer->setInterval(3000);
    connect(m_checkTimer, &QTimer::timeout, this, &AppMgr::checkPendingAppItems);
}

AppMgr *AppMgr::instance()
{
    static AppMgr appMgr;
    return &appMgr;
}

bool AppMgr::launchApp(const QString &desktopId, const QString &activationToken)
{
    qCInfo(logAppMgr) << "Launching app:" << desktopId;
    const QString path = applicationObjectPath(desktopId);
    if (path.isEmpty()) {
        qCWarning(logAppMgr) << "Failed to get path for desktop ID:" << desktopId;
        return false;
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);

    QStringList args = {"--by-user", path};
#ifdef HAVE_DDE_API_EVENTLOGGER
    args << "--launch-type" << "dde-launchpad";
#endif

    if (!activationToken.isEmpty()) {
        qCDebug(logAppMgr) << "Passing XDG_ACTIVATION_TOKEN to dde-am for:" << desktopId;
        args << "--env" << (QLatin1String("XDG_ACTIVATION_TOKEN=") + activationToken);
    }

    process.start("dde-am", args);
    if (!process.waitForFinished()) {
        qCWarning(logAppMgr) << "Failed to launch the desktopId:" << desktopId << process.errorString();
        return false;
    } else if (process.exitCode() != 0) {
        qCWarning(logAppMgr) << "Failed to launch the desktopId:" << desktopId << process.readAll();
        return false;
    }

    qCInfo(logAppMgr) << "Successfully launched desktop ID:" << desktopId;
    return true;
}

bool AppMgr::autoStart(const QString &desktopId)
{
    const QVariant value = instance()->readProperty(desktopId, QStringLiteral("AutoStart"));
    return value.isValid() && value.toBool();
}

void AppMgr::setAutoStart(const QString &desktopId, bool enabled)
{
    instance()->setProperty(desktopId, QStringLiteral("AutoStart"), enabled);
}

bool AppMgr::disableScale(const QString &desktopId)
{
    const QVariant environment = instance()->readProperty(desktopId, QStringLiteral("Environ"));
    return environment.isValid() && isScaleDisabled(environment.toString());
}

void AppMgr::setDisableScale(const QString &desktopId, bool disabled)
{
    const QVariant environmentValue = instance()->readProperty(desktopId, QStringLiteral("Environ"));
    if (!environmentValue.isValid())
        return;

    const QString environment = environmentValue.toString();
    const QString updatedEnvironment = scaleEnvironment(environment, disabled);
    if (updatedEnvironment != environment)
        instance()->setProperty(desktopId, QStringLiteral("Environ"), updatedEnvironment);
}

QVariant AppMgr::readProperty(const QString &desktopId, const QString &property) const
{
    const QString path = applicationObjectPath(desktopId);
    if (path.isEmpty()) {
        qCWarning(logAppMgr) << "Invalid desktop ID for property" << property << ":" << desktopId;
        return {};
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        ApplicationManagerService, path, PropertiesInterface, QStringLiteral("Get"));
    message.setArguments({ApplicationInterface, property});
    const QDBusReply<QDBusVariant> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid()) {
        qCWarning(logAppMgr) << "Failed to read property" << property << "for" << desktopId << reply.error();
        return {};
    }
    return reply.value().variant();
}

bool AppMgr::setProperty(const QString &desktopId, const QString &property, const QVariant &value)
{
    const QString path = applicationObjectPath(desktopId);
    if (path.isEmpty()) {
        qCWarning(logAppMgr) << "Invalid desktop ID for property" << property << ":" << desktopId;
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        ApplicationManagerService, path, PropertiesInterface, QStringLiteral("Set"));
    message.setArguments({ApplicationInterface, property, QVariant::fromValue(QDBusVariant(value))});
    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(logAppMgr) << "Failed to set property" << property << "for" << desktopId
                             << reply.errorName() << reply.errorMessage();
        return false;
    }
    return true;
}

bool AppMgr::callBoolMethod(const QString &desktopId, const QString &method) const
{
    const QString path = applicationObjectPath(desktopId);
    if (path.isEmpty()) {
        qCWarning(logAppMgr) << "Invalid desktop ID for" << method << ":" << desktopId;
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(ApplicationManagerService, path, ApplicationInterface, method);
    const QDBusReply<bool> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid()) {
        qCWarning(logAppMgr) << method << "failed for" << desktopId << reply.error();
        return false;
    }
    return reply.value();
}

bool AppMgr::sendToDesktop(const QString &desktopId)
{
    return instance()->callBoolMethod(desktopId, QStringLiteral("SendToDesktop"));
}

bool AppMgr::removeFromDesktop(const QString &desktopId)
{
    return instance()->callBoolMethod(desktopId, QStringLiteral("RemoveFromDesktop"));
}

 // For: bug-347859
bool AppMgr::waitForIcon(const QString &desktopId, const QString &iconName)
{
    if (desktopId.isEmpty() || !QFileInfo(iconName).isAbsolute() || QFileInfo::exists(iconName)) {
        cancelPendingAppItem(desktopId);
        return false;
    }

    if (!m_pendingAppItems.contains(desktopId))
        m_checkCount = 0;
    m_pendingAppItems.insert(desktopId, iconName);

    if (!m_checkTimer->isActive())
        m_checkTimer->start();
    return true;
}

void AppMgr::cancelPendingAppItem(const QString &desktopId)
{
    m_pendingAppItems.remove(desktopId);
    if (m_pendingAppItems.isEmpty()) {
        m_checkTimer->stop();
        m_checkCount = 0;
    }
}

void AppMgr::clearPendingAppItems()
{
    m_pendingAppItems.clear();
    m_checkTimer->stop();
    m_checkCount = 0;
}

bool AppMgr::isPendingAppItem(const QString &desktopId) const
{
    return m_pendingAppItems.contains(desktopId);
}

void AppMgr::checkPendingAppItems()
{
    ++m_checkCount;

    QStringList readyItems;
    for (auto it = m_pendingAppItems.begin(); it != m_pendingAppItems.end();) {
        if (QFileInfo::exists(it.value())) {
            readyItems.append(it.key());
            it = m_pendingAppItems.erase(it);
        } else {
            ++it;
        }
    }

    if (m_checkCount >= 20) {
        readyItems.append(m_pendingAppItems.keys());
        m_pendingAppItems.clear();
    }

    if (m_pendingAppItems.isEmpty()) {
        m_checkTimer->stop();
        m_checkCount = 0;
    }

    for (const QString &desktopId : std::as_const(readyItems))
        Q_EMIT pendingAppItemReady(desktopId);
}
