# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt6Core CONFIG REQUIRED Private)

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
    sortproxymodel.h
    searchfilterproxymodel.h
    categorizedsortproxymodel.h
    favoritedproxymodel.h
    itemarrangementproxymodel.h
    multipagesortfilterproxymodel.h
    recentlyinstalledproxymodel.h
    countlimitproxymodel.h
    freesortproxymodel.h
    frequentlyusedproxymodel.h
)

target_sources(launcher-models
PRIVATE
    appsmodel.cpp
    appitem.cpp appitem.h
    sortproxymodel.cpp
    searchfilterproxymodel.cpp
    categorizedsortproxymodel.cpp
    favoritedproxymodel.cpp
    itemspage.cpp itemspage.h
    itemarrangementproxymodel.cpp
    multipagesortfilterproxymodel.cpp
    recentlyinstalledproxymodel.cpp
    countlimitproxymodel.cpp
    freesortproxymodel.cpp
    frequentlyusedproxymodel.cpp
)

target_link_libraries(launcher-models PRIVATE
    Qt::Core
    Qt::Gui
    Qt::CorePrivate
    ${DTK_NS}::Core

    gio-utils
    dde-integration-dbus
    launcher-utils
)

dtk_add_config_meta_files(
  APPID ${PROJECT_NAME}
  FILES org.deepin.dde.launchpad.appsmodel.json
)
