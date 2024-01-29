# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

if(NOT DEFINED VERSION)
    set(VERSION 0.4.6)
endif()

project(dde-launchpad VERSION ${VERSION})

set(CMAKE_CXX_STANDARD 17) # blurhash requires 17, otherwish we can still use 14
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # ensure adapter class can include launchercontroller.h

set(BIN_NAME dde-launchpad)

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_NS Qt6)
set(DTK_NS Dtk6)
set(ASQT_NS AppStreamQt)

find_package(QT NAMES ${QT_NS} REQUIRED COMPONENTS Core)
find_package(${QT_NS} REQUIRED COMPONENTS Core Gui Concurrent Qml Svg Quick QuickControls2 LinguistTools)
find_package(${DTK_NS} REQUIRED COMPONENTS Core Gui)
find_package(${ASQT_NS} 1.0)

set_package_properties(${ASQT_NS} PROPERTIES
    DESCRIPTION "Library that lists Appstream resources"
    URL "https://www.freedesktop.org"
    TYPE RECOMMENDED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(systemd)
add_subdirectory(dbus)
add_subdirectory(src/gioutils)
add_subdirectory(src/utils)
add_subdirectory(src/quick)
add_subdirectory(src/ddeintegration)
add_subdirectory(src/models)

set(SOURCE_FILES
    main.cpp
    desktopintegration.cpp desktopintegration.h
    launchercontroller.cpp launchercontroller.h
    debughelper.cpp debughelper.h
)

set(QML_FILES
    qml/Main.qml
    qml/WindowedFrame.qml
    qml/FullscreenFrame.qml
    qml/AppItemMenu.qml
    qml/AppListView.qml
)

set(TRANSLATION_FILES
    translations/dde-launchpad.ts
    translations/dde-launchpad_az.ts
    translations/dde-launchpad_bo.ts
    translations/dde-launchpad_ca.ts
    translations/dde-launchpad_es.ts
    translations/dde-launchpad_fi.ts
    translations/dde-launchpad_fr.ts
    translations/dde-launchpad_hu.ts
    translations/dde-launchpad_it.ts
    translations/dde-launchpad_ja.ts
    translations/dde-launchpad_ko.ts
    translations/dde-launchpad_nb_NO.ts
    translations/dde-launchpad_pl.ts
    translations/dde-launchpad_pt_BR.ts
    translations/dde-launchpad_ru.ts
    translations/dde-launchpad_uk.ts
    translations/dde-launchpad_zh_CN.ts
    translations/dde-launchpad_zh_HK.ts
    translations/dde-launchpad_zh_TW.ts
)

qt_add_dbus_adaptor(DBUS_ADAPTER_FILES dbus/org.deepin.dde.Launcher1.xml launchercontroller.h LauncherController)

if(${QT_VERSION_MAJOR} EQUAL "5")
    qtquick_compiler_add_resources(RESOURCES qml.qrc)
else()
    qt_add_resources(RESOURCES qml.qrc) # TODO: should use qt_add_qml_module instead
endif()

add_executable(${BIN_NAME}
    ${SOURCE_FILES}
    ${DBUS_ADAPTER_FILES}
    ${RESOURCES}
    ${TRANSLATED_FILES}
)

qt_add_translations(${BIN_NAME}
    TS_FILES ${TRANSLATION_FILES}
    SOURCES ${QML_FILES} ${SOURCE_FILES} src/models/appitem.cpp
    QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
)

target_compile_definitions(${BIN_NAME}
PRIVATE
    DDE_LAUNCHPAD_VERSION=${CMAKE_PROJECT_VERSION}
)

target_link_libraries(${BIN_NAME} PRIVATE
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

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)
install(
    FILES dist/org.deepin.dde.launchpad.appdata.xml
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
)
