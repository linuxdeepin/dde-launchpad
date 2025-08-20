// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddedock.h"

#include <QDBusConnection>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDdeIntegration)

#include "DaemonDock1.h"

using Dock1 = __Dock1;

DdeDock::DdeDock(QObject *parent)
    : QObject(parent)
    , m_dbusDaemonDockIface(new Dock1(Dock1::staticInterfaceName(), "/org/deepin/dde/daemon/Dock1",
                                      QDBusConnection::sessionBus(), this))
    , m_direction(Qt::DownArrow)
    , m_rect(QRect(-1, -1, 0, 0))
{
    QTimer::singleShot(0, this, &DdeDock::updateDockRectAndPositionFromDBus);

    // Due to current dde-dock bug, we need to do both since position changed signal might not got emit in some case.
    connect(m_dbusDaemonDockIface, &Dock1::PositionChanged, this, &DdeDock::updateDockRectAndPositionFromDBus);
    connect(m_dbusDaemonDockIface, &Dock1::FrontendWindowRectChanged, this, &DdeDock::updateDockRectAndPositionFromDBus);
    qCInfo(logDdeIntegration) << "DdeDock initialized with signals connected";
}

DdeDock::~DdeDock()
{
    qCDebug(logDdeIntegration) << "Destroying DdeDock";
}

Qt::ArrowType DdeDock::direction() const
{
    return m_direction;
}

QRect DdeDock::geometry() const
{
    return m_rect;
}

bool DdeDock::isDocked(const QString &desktop) const
{
    QDBusPendingReply<bool> reply(m_dbusDaemonDockIface->IsDocked(desktop));
    reply.waitForFinished();

    if (reply.isError()) {
        qCWarning(logDdeIntegration) << "D-Bus error checking dock status:" << reply.error();
        return false;
    }

    const bool result = reply.value();
    qCDebug(logDdeIntegration) << "App" << desktop << "dock status:" << result;
    return result;
}

void DdeDock::sendToDock(const QString &desktop, int idx)
{
    qCInfo(logDdeIntegration) << "Sending app to dock:" << desktop << "at index:" << idx;
    m_dbusDaemonDockIface->RequestDock(desktop, idx);
}

void DdeDock::removeFromDock(const QString &desktop)
{
    qCInfo(logDdeIntegration) << "Removing app from dock:" << desktop;
    m_dbusDaemonDockIface->RequestUndock(desktop);
}

void DdeDock::updateDockRectAndPositionFromDBus()
{
    updateDockPositionFromDBus();
    updateDockRectFromDBus();
}

void DdeDock::updateDockPositionFromDBus()
{
    Qt::ArrowType newDirection = Qt::NoArrow;
    enum DockPosition {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

    switch (m_dbusDaemonDockIface->position()) {
    case Top:
        qCDebug(logDdeIntegration) << "Setting direction to UpArrow (Top)";
        newDirection = Qt::UpArrow;
        break;
    case Right:
        qCDebug(logDdeIntegration) << "Setting direction to RightArrow (Right)";
        newDirection = Qt::RightArrow;
        break;
    case Bottom:
        qCDebug(logDdeIntegration) << "Setting direction to DownArrow (Bottom)";
        newDirection = Qt::DownArrow;
        break;
    case Left:
        qCDebug(logDdeIntegration) << "Setting direction to LeftArrow (Left)";
        newDirection = Qt::LeftArrow;
        break;
    }

    if (newDirection != m_direction) {
        qCInfo(logDdeIntegration) << "Dock direction changed from" << static_cast<int>(m_direction) << "to" << static_cast<int>(newDirection);
        m_direction = newDirection;
        emit directionChanged();
    }
}

void DdeDock::updateDockRectFromDBus()
{
    m_rect = m_dbusDaemonDockIface->frontendWindowRect();
    qCInfo(logDdeIntegration) << "Updated dock rect from D-Bus:" << m_rect;
    emit geometryChanged();
}
