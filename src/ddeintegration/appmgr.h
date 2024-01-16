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
    static bool autoStart(const QString & desktopId);
    static void setAutoStart(const QString & desktopId, bool autoStart);
    static double scaleFactor(const QString & desktopId);
    static void setScaleFactor(const QString & desktopId, double scaleFactor);
};
