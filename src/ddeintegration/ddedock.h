// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRect>

class __Dock1;
class DdeDock : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Qt::ArrowType direction READ direction NOTIFY directionChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)

public:
    explicit DdeDock(QObject *parent = nullptr);
    ~DdeDock();

    Qt::ArrowType direction() const;
    QRect geometry() const;

    bool isDocked(const QString & desktop) const;
    void sendToDock(const QString & desktop, int idx = -1);
    void removeFromDock(const QString & desktop);

signals:
    void directionChanged();
    void geometryChanged();

private:
    void updateDockRectAndPositionFromDBus();
    void updateDockPositionFromDBus();
    void updateDockRectFromDBus();

    __Dock1 * m_dbusDaemonDockIface;

    Qt::ArrowType m_direction;
    QRect m_rect;
};
