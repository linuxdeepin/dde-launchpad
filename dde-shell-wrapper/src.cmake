# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(DDEShell)
if (DDEShell_FOUND)

    add_library(ds-launchpad-wrapper SHARED
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${SOURCE_FILES}
        ${DBUS_ADAPTER_FILES}
        ${RESOURCES}
        ${TRANSLATED_FILES}
    )
    target_compile_definitions(ds-launchpad-wrapper
    PRIVATE
        DDE_LAUNCHPAD_VERSION=${CMAKE_PROJECT_VERSION}
    )
    target_link_libraries(ds-launchpad-wrapper PRIVATE
        Dde::Shell

        ${DTK_NS}::Core
        ${DTK_NS}::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2

        gio-utils
        launcher-utils
        launcher-qml-utils
        launcher-models
        dde-integration-dbus
    )
    ds_install_package(PACKAGE org.deepin.ds.launchpad TARGET ds-launchpad-wrapper PACKAGE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/package)

endif()
