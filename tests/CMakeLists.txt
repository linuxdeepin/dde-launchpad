# SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(Qt6 REQUIRED COMPONENTS Gui Qml Svg)  # needed by tested-source OBJECT libs
find_package(Qt6Core CONFIG REQUIRED Private)      # sortproxymodel.cpp uses QAbstractItemModelPrivate

qt_standard_project_setup()

# Coverage instrumentation (off by default; enable with -DENABLE_COVERAGE=ON
# so `coverage` produces a report).
# Only the test-directory targets (OBJECT libs + test executables) are affected;
# the production `launchpadcommon` shared library is left untouched.
option(ENABLE_COVERAGE "Enable coverage instrumentation for tests" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(-no-pie)
    link_libraries(-lgcov)
    # The gio-utils OBJECT library is defined in src/gioutils/CMakeLists.txt,
    # outside this directory scope, so add_compile_options above does not
    # instrument it.  Add coverage flags explicitly so src/gioutils/*.cpp
    # (appinfomonitor.cpp, trashmonitor.cpp, appinfo.cpp) generate .gcno/.gcda
    # files and contribute to the coverage report.
    # CMP0079 allows target_link_libraries on a target created in another
    # directory scope; needed because gio-utils is defined in src/gioutils/.
    cmake_policy(SET CMP0079 NEW)
    target_compile_options(gio-utils PRIVATE --coverage)
    target_link_libraries(gio-utils PUBLIC -lgcov)
endif()

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
        # Only itemarrangementproxymodeltest and itemspagemodeltest use
        # foldermodelstub.h; list it there so AUTOMOC generates its moc,
        # and avoid compiling a useless moc file for the other 14 targets.
        if(_testname STREQUAL "itemarrangementproxymodeltest" OR _testname STREQUAL "itemspagemodeltest")
            qt_add_executable(launchpad-${_testname} ${_testname}.cpp foldermodelstub.h)
        else()
            qt_add_executable(launchpad-${_testname} ${_testname}.cpp)
        endif()
        # Test sources include the headers under test by bare name (e.g.
        # `#include "categoryutils.h"`), matching how src/ includes its own
        # headers — no `../src/...` relative paths.
        target_include_directories(launchpad-${_testname} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/models
            ${CMAKE_SOURCE_DIR}/src/utils
            ${CMAKE_SOURCE_DIR}/src/gioutils
        )
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
        # debian/rules sets CMAKE_SKIP_BUILD_RPATH=ON, so the build-tree rpath to
        # launchpadcommon.so is absent; point the loader at it explicitly.
        # Use a generator expression so the path is resolved at build time, not
        # configure time (avoids capturing a stale $ENV{LD_LIBRARY_PATH}).
        set_tests_properties(launchpad-${_testname} PROPERTIES
            ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:launchpadcommon>;QT_QPA_PLATFORM=offscreen")
        list(APPEND LAUNCHPAD_TEST_TARGETS launchpad-${_testname})
    endforeach()
endmacro()

launchpad_add_tests(
    itemspagetest
    itemarrangementproxytest
    appinfomonitortest
    trashmonitortest
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

# Expose private/protected members for tests that need direct access to
# private methods (onAppInfoMonitorChanged, onTrashMonitorChanged, etc.).
# -fno-access-control disables C++ access checking for the test compilation
# unit, achieving the same effect as macro replacement without breaking
# standard library headers like <sstream>.
target_compile_options(launchpad-appinfomonitortest PRIVATE -fno-access-control)
target_compile_options(launchpad-trashmonitortest PRIVATE -fno-access-control)

# gioappinfotest looks up a desktop file through GIO, which searches
# XDG_DATA_HOME.  GIO caches those directories early in process startup, so the
# fixture must be in the environment before the test is launched, not set from
# inside the test.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gio-fixture/applications)
# GIO drops desktop entries whose Exec program does not exist, and the build
# chroot does not have dde-file-manager installed, so point Exec at /bin/true.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gio-fixture/applications/dde-file-manager.desktop
     "[Desktop Entry]\nType=Application\nName=File Manager\nExec=/bin/true\n")

# Additional fixture desktop file for appinfomonitortest: covers the
# X-Deepin-Vendor=deepin branch (displayName replaced by GenericName),
# icon non-null path, and keywords (fromGStrV non-null) path.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gio-fixture/applications/dde-test-vendor.desktop
     "[Desktop Entry]\nType=Application\nName=Test Vendor\nGenericName=Test Generic\nExec=/bin/true\nIcon=folder\nX-Deepin-Vendor=deepin\nKeywords=test;vendor;\n")
set_tests_properties(launchpad-gioappinfotest PROPERTIES ENVIRONMENT
    "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:launchpadcommon>;QT_QPA_PLATFORM=offscreen;XDG_DATA_HOME=${CMAKE_CURRENT_BINARY_DIR}/gio-fixture")

# appinfomonitortest also uses the GIO fixture (XDG_DATA_HOME) so that
# allAppInfosShouldBeShown() finds the fixture desktop files and exercises
# the iteration / icon / vendor / keywords code paths.
set_tests_properties(launchpad-appinfomonitortest PROPERTIES ENVIRONMENT
    "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:launchpadcommon>;QT_QPA_PLATFORM=offscreen;XDG_DATA_HOME=${CMAKE_CURRENT_BINARY_DIR}/gio-fixture")

# ---- coverage: execute all tests and generate a coverage report ----
# ---- run-test: run all tests without coverage instrumentation ----
add_custom_target(run-test
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${LAUNCHPAD_TEST_TARGETS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests"
    VERBATIM
)

# Produces: ${CMAKE_BINARY_DIR}/coverage_report/index.html
find_program(LCOV_BIN lcov)
find_program(GENHTML_BIN genhtml)
set(_run_test_coverage "")
if(LCOV_BIN AND GENHTML_BIN)
    set(_run_test_coverage
        COMMAND ${LCOV_BIN} --capture --directory ${CMAKE_BINARY_DIR}
                --rc lcov_branch_coverage=1 --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${LCOV_BIN} --extract ${CMAKE_BINARY_DIR}/coverage.info
                --rc lcov_branch_coverage=1
                "${CMAKE_SOURCE_DIR}/src/models/*"
                "${CMAKE_SOURCE_DIR}/src/utils/*"
                "${CMAKE_SOURCE_DIR}/src/gioutils/*"
                --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
        COMMAND ${GENHTML_BIN} ${CMAKE_BINARY_DIR}/coverage_filtered.info
                --branch-coverage --output-directory ${CMAKE_BINARY_DIR}/coverage_report
    )
endif()
add_custom_target(coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    ${_run_test_coverage}
    DEPENDS ${LAUNCHPAD_TEST_TARGETS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests and generate coverage report"
    VERBATIM
)
