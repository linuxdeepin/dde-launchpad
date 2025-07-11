# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt6 REQUIRED COMPONENTS Core Test)

qt_standard_project_setup()

macro(launchpad_add_tests)
    foreach(_testname ${ARGN})
        qt_add_executable(launchpad-${_testname} ${_testname}.cpp)
        target_link_libraries(launchpad-${_testname} PRIVATE Qt6::Core Qt6::Test launchpadcommon)
        add_test(NAME launchpad-${_testname} COMMAND launchpad-${_testname})
    endforeach()
endmacro()

launchpad_add_tests(
    itemspagetest
    gioappinfotest
    searchfilterproxymodeltest
)
