# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt5 REQUIRED Core)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0 gio-unix-2.0)

add_library(gio-utils OBJECT)

target_sources(gio-utils
PRIVATE
    trashmonitor.h
    trashmonitor.cpp
    appinfomonitor.h
    appinfomonitor.cpp
    appinfo.h
    appinfo.cpp
)

target_include_directories(gio-utils PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(gio-utils PUBLIC
    Qt::Core
    PkgConfig::GIO
)

target_compile_definitions(gio-utils
PRIVATE
    QT_NO_KEYWORDS
)
