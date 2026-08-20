# SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(Qt6 REQUIRED COMPONENTS Gui Qml Svg)  # needed by tested-source OBJECT libs
find_package(Qt6Core CONFIG REQUIRED Private)      # sortproxymodel.cpp uses QAbstractItemModelPrivate

qt_standard_project_setup()

# ---------------------------------------------------------------------------
# Tested-source OBJECT libraries
#
# Per the long-term coverage rule: the tested source files (src/models/*.cpp,
# src/utils/*.cpp) are compiled as OBJECT libraries and linked *directly*
# into every test executable, instead of being reached only through the
# launchpadcommon.so shared library.  This ensures gcov .gcda runtime data
# is written to the test build directory and can be captured even in
# cross-prefix environments where the shared-library build path is not
# writable at runtime.
#
# launchpadcommon is still linked afterwards for transitive dependencies
# (D-Bus adaptors, dde-integration AppMgr, etc.).  Symbols defined by the
# OBJECT libs take precedence over the same symbols in the shared library,
# so the gcov-instrumented copies in the test executable are the ones that
# actually execute.
# ---------------------------------------------------------------------------

add_library(launchpad-test-models OBJECT
    ${CMAKE_SOURCE_DIR}/src/models/appsmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/categorizedsortproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/countlimitproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/favoritedproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/freesortproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/frequentlyusedproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/itemarrangementproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/itemspage.cpp
    ${CMAKE_SOURCE_DIR}/src/models/itemspagemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/multipagesortfilterproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/recentlyinstalledproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/searchfilterproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/sortproxymodel.cpp
)
target_include_directories(launchpad-test-models PRIVATE
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/ddeintegration
)
target_link_libraries(launchpad-test-models PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml        # QML_NAMED_ELEMENT / QML_SINGLETON in model headers (moc-generated registration)
    Qt6::CorePrivate # sortproxymodel.cpp -> QAbstractItemModelPrivate
    ${DTK_NS}::Core  # DConfig, DFileWatcherManager, DPinyin (appsmodel.cpp, frequentlyusedproxymodel.cpp)
)

add_library(launchpad-test-utils OBJECT
    ${CMAKE_SOURCE_DIR}/src/utils/blurhash.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/categoryutils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/iconutils.cpp
)
target_include_directories(launchpad-test-utils PRIVATE
    ${CMAKE_SOURCE_DIR}/src/utils
)
target_link_libraries(launchpad-test-utils PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    ${DTK_NS}::Gui   # DIcon, DIconTheme, DSvgRenderer (iconutils.cpp)
)

macro(launchpad_add_tests)
    foreach(_testname ${ARGN})
        qt_add_executable(launchpad-${_testname} ${_testname}.cpp)
        # OBJECT libs first: their symbols take precedence over launchpadcommon.so,
        # so the gcov-instrumented copies are the ones that execute at test time.
        target_link_libraries(launchpad-${_testname} PRIVATE
            Qt6::Core
            Qt6::Test
            launchpad-test-models
            launchpad-test-utils
            gio-utils          # already OBJECT; covers src/gioutils/ (e.g. gioappinfotest)
            launchpadcommon
        )
        add_test(NAME launchpad-${_testname} COMMAND launchpad-${_testname})
    endforeach()
endmacro()

launchpad_add_tests(
    itemspagetest
    gioappinfotest
    searchfilterproxymodeltest
    countlimitproxymodeltest
    sortproxymodeltest
    freesortproxymodeltest
    recentlyinstalledproxymodeltest
    multipagesortfilterproxymodeltest
    categoryutilstest
    blurhashtest
    favoritedproxymodeltest
    frequentlyusedproxymodeltest
    itemarrangementproxymodeltest
    iconutilstest
    itemspagemodeltest
)
