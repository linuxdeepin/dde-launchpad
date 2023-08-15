// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class AppInfo
{
public:
    static bool launchByDesktopId(const QString & desktopId);
    static QString fullPathByDesktopId(const QString & desktopId);
};
