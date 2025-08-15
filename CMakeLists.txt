# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.10)

project(dde-launchpad VERSION 0.7.0)

option(BUILD_TEST "Whether or not to build the tests" OFF)
option(CMAKE_EXPORT_COMPILE_COMMANDS "clangd support" ON)

set(CMAKE_CXX_STANDARD 17) # blurhash requires 17, otherwish we can still use 14
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # ensure adapter class can include launchercontroller.h
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_NS Qt6)
set(DTK_NS Dtk6)
set(ASQT_NS AppStreamQt)

find_package(QT NAMES ${QT_NS} REQUIRED COMPONENTS Core)
find_package(${QT_NS} REQUIRED COMPONENTS Core Gui Concurrent Qml Svg Quick QuickControls2 LinguistTools)
find_package(${DTK_NS} REQUIRED COMPONENTS Core Gui)
find_package(${ASQT_NS} 1.0 REQUIRED)
find_package(ECM NO_MODULE)

set_package_properties(${ASQT_NS} PROPERTIES
    DESCRIPTION "Library that lists Appstream resources"
    URL "https://www.freedesktop.org"
    TYPE RECOMMENDED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(QML_IMPORT_PATH ${QML_IMPORT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/ CACHE STRING "" FORCE)

if (ECM_FOUND)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};${ECM_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

    include(KDEClangFormat)
    include(KDEGitCommitHooks)
endif()

add_subdirectory(src/gioutils)
add_subdirectory(src/utils)
add_subdirectory(src/quick)
add_subdirectory(src/ddeintegration)
add_subdirectory(src/treelandintegration)
add_subdirectory(src/models)
add_subdirectory(qml/windowed)

if(BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()

set(SOURCE_FILES
    desktopintegration.cpp desktopintegration.h
    launchercontroller.cpp launchercontroller.h
    debughelper.cpp debughelper.h
    inputeventitem.h inputeventitem.cpp
)

set(QML_FILES
    ${PROJECT_SOURCE_DIR}/qml/Helper.qml
    ${PROJECT_SOURCE_DIR}/qml/Main.qml
    ${PROJECT_SOURCE_DIR}/qml/FullscreenFrame.qml
    ${PROJECT_SOURCE_DIR}/qml/AppItemMenu.qml
    ${PROJECT_SOURCE_DIR}/qml/DummyAppItemMenu.qml
    ${PROJECT_SOURCE_DIR}/qml/GridViewContainer.qml
    ${PROJECT_SOURCE_DIR}/qml/DrawerFolder.qml
    ${PROJECT_SOURCE_DIR}/qml/IconItemDelegate.qml
    ${PROJECT_SOURCE_DIR}/qml/DebugDialog.qml
    ${PROJECT_SOURCE_DIR}/qml/DebugBounding.qml
    ${PROJECT_SOURCE_DIR}/qml/FolderGridViewPopup.qml
)
foreach(QML_FILE ${QML_FILES})
    get_filename_component(file_name ${QML_FILE} NAME)
    set_source_files_properties(${QML_FILE}
        PROPERTIES QT_RESOURCE_ALIAS ${file_name}
    )
endforeach()

get_target_property(WINDOWED_QML_FILES launcher-qml-windowed QT_QML_MODULE_QML_FILES)
# TODO: switch to the new qt_add_translations usage introduced in Qt 6.7
#       see also: https://www.qt.io/blog/revisited-i18n-with-cmake
set(QML_FILES_NEED_TRANSLATION
    ${PROJECT_SOURCE_DIR}/qml/AppItemMenu.qml
    ${PROJECT_SOURCE_DIR}/qml/DummyAppItemMenu.qml
    ${PROJECT_SOURCE_DIR}/qml/Main.qml
    ${PROJECT_SOURCE_DIR}/qml/FullscreenFrame.qml
    ${WINDOWED_QML_FILES}
)

set_source_files_properties(${PROJECT_SOURCE_DIR}/qml/Helper.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_dbus_adaptor(DBUS_ADAPTER_FILES dbus/org.deepin.dde.Launcher1.xml launchercontroller.h LauncherController)

qt_add_library(launchpadcommon SHARED
    ${SOURCE_FILES}
    ${DBUS_ADAPTER_FILES}
    ${RESOURCES}
    ${TRANSLATED_FILES}
)

qt_add_qml_module(launchpadcommon
    URI org.deepin.launchpad
    VERSION 1.0
    RESOURCES qml.qrc
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        ${QML_FILES}
)
set_target_properties(launchpadcommon PROPERTIES PREFIX "")

target_link_libraries(launchpadcommon PUBLIC
    ${DTK_NS}::Core
    ${DTK_NS}::Gui
    Qt::GuiPrivate
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2

    launcher-qml-windowed
    gio-utils
    launcher-utils
    launcher-qml-utils
    launcher-models
    dde-integration-dbus
    treeland-integration
)

target_link_options(launchpadcommon PRIVATE
    -Wl,--no-undefined
    -Wl,--build-id=none
    -Wl,--sort-section=alignment
)

install(TARGETS launchpadcommon DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(
    FILES dist/org.deepin.dde.shell.launchpad.appdata.xml
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
)

add_subdirectory(shell-launcher-applet)
