# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

qt_add_qml_module(
    launcher-models
    URI org.deepin.launchpad.models
    VERSION 1.0
    STATIC
    PLUGIN_TARGET launcher-models
)

target_sources(launcher-models PUBLIC
FILE_SET HEADERS
FILES
    appsmodel.h
    searchfilterproxymodel.h
    categorizedsortproxymodel.h
    favoritedproxymodel.h
    multipageproxymodel.h
    multipagesortfilterproxymodel.h
    recentlyinstalledproxymodel.h
    countlimitproxymodel.h
)

target_sources(launcher-models
PRIVATE
    appsmodel.cpp
    appitem.cpp appitem.h
    searchfilterproxymodel.cpp
    categorizedsortproxymodel.cpp
    favoritedproxymodel.cpp
    itemspage.cpp itemspage.h
    multipageproxymodel.cpp
    multipagesortfilterproxymodel.cpp
    recentlyinstalledproxymodel.cpp
    countlimitproxymodel.cpp
)

target_link_libraries(launcher-models PRIVATE
    Qt::Core
    Qt::Gui
    ${DTK_NS}::Core

    gio-utils
    dde-integration-dbus
    launcher-utils
)

dtk_add_config_meta_files(
  APPID ${PROJECT_NAME}
  FILES org.deepin.dde.launchpad.appsmodel.json
)
