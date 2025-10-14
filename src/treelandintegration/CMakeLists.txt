# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt6 REQUIRED GLOBAL COMPONENTS WaylandClient)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
  find_package(Qt6 COMPONENTS WaylandClientPrivate REQUIRED)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(treeland-integration OBJECT
    personalizationmanager.cpp
    personalizationwindow.cpp
)

qt_generate_wayland_protocol_client_sources(treeland-integration
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/treeland-personalization-manager-v1.xml
)

target_sources(treeland-integration PUBLIC
FILE_SET HEADERS
FILES
    personalizationmanager.h
    personalizationwindow.h
)

target_link_libraries(treeland-integration
PRIVATE
    Qt6::WaylandClient
    Qt6::WaylandClientPrivate
)
