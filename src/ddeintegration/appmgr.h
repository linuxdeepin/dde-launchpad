// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class AppMgr : public QObject
{
    Q_OBJECT
public:
    explicit AppMgr(QObject *parent = nullptr);
    ~AppMgr();

    static bool launchApp(const QString & desktopId);
};
