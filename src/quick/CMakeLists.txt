# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt5 REQUIRED Quick)

add_library(launcher-qml-utils OBJECT)

target_sources(launcher-qml-utils
PRIVATE
    launcherappiconprovider.h
    launcherappiconprovider.cpp
    blurhashimageprovider.h
    blurhashimageprovider.cpp
)

target_include_directories(launcher-qml-utils PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(launcher-qml-utils PRIVATE Qt::Quick launcher-utils)
