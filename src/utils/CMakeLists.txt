# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt NAMES ${QT_NS} REQUIRED Quick)
find_package(Dtk NAMES ${DTK_NS} REQUIRED COMPONENTS Gui) # for DHiDPIHelper, probably can be replaced later

add_library(launcher-utils OBJECT)

target_sources(launcher-utils
PRIVATE
    categoryutils.h
    categoryutils.cpp
    iconutils.h
    iconutils.cpp
    blurhash.hpp
    blurhash.cpp
)

target_include_directories(launcher-utils PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(launcher-utils PRIVATE Qt::Core Qt::Gui Qt::Svg ${DTK_NS}::Gui)
