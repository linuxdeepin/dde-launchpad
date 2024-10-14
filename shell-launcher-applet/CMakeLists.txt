# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(DDEShell REQUIRED)

add_library(shell-launcherapplet SHARED
    launcheritem.cpp
    launcheritem.h
)

target_link_libraries(shell-launcherapplet PRIVATE
    Dde::Shell
    launchpadcommon
)

ds_install_package(PACKAGE org.deepin.ds.dock.launcherapplet TARGET shell-launcherapplet)
ds_handle_package_translation(
PACKAGE org.deepin.ds.dock.launcherapplet
QML_FILES ${QML_FILES_NEED_TRANSLATION}
)
