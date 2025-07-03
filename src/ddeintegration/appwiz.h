// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class __DaemonLauncher1;
class AppWiz : public QObject
{
    Q_OBJECT

public:
    explicit AppWiz(QObject *parent = nullptr);
    ~AppWiz();

    void legacyRequestUninstall(const QString & desktopFileFullPath);

private:
    void updateCurrentWallpaperBlurhash();

    __DaemonLauncher1 * m_dbusDaemonLauncherIface;
};
