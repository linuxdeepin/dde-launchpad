// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtWaylandClient/QWaylandClientExtension>

class QWindow;
class PersonalizationManagerPrivate;
class PersonalizationManager : public QObject
{
    Q_OBJECT
public:
    enum BgState {
        BgNormal = 0,
        BgWallpaper = 1,
        BgBlurredWallpaper = 2,
    };

    explicit PersonalizationManager(QObject * parent = nullptr);
    ~PersonalizationManager();
    bool personalizeWindow(QWindow * window, BgState state);

private:
    PersonalizationManagerPrivate * m_dptr = nullptr;
};
