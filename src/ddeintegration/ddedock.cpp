// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddedock.h"

#include <QDBusConnection>

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
}

DdeDock::~DdeDock()
{

}

Qt::ArrowType DdeDock::direction() const
{
    return m_direction;
}

QRect DdeDock::geometry() const
{
    return m_rect;
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
        newDirection = Qt::UpArrow;
        break;
    case Right:
        newDirection = Qt::RightArrow;
        break;
    case Bottom:
        newDirection = Qt::DownArrow;
        break;
    case Left:
        newDirection = Qt::LeftArrow;
        break;
    }

    if (newDirection != m_direction) {
        m_direction = newDirection;
        emit directionChanged();
    }
}

void DdeDock::updateDockRectFromDBus()
{
    m_rect = m_dbusDaemonDockIface->frontendWindowRect();
    emit geometryChanged();
}
