# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

add_library(launcher-models)

target_sources(launcher-models PUBLIC
FILE_SET HEADERS
FILES
    appsmodel.h
    searchfilterproxymodel.h
    categorizedsortproxymodel.h
    favoritedproxymodel.h
    multipageproxymodel.h
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
)

target_link_libraries(launcher-models PRIVATE
    Qt::Core
    Qt::Gui
    ${DTK_NS}::Core

    gio-utils
    launcher-utils
)
