# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Dtk REQUIRED COMPONENTS Tools) # for ${DTK_XML2CPP} which provides the path of `qdbusxml2cpp-fix` binary

function(dtk_add_dbus_interface _sources _interface _relativename)
    get_filename_component(_infile ${_interface} ABSOLUTE)
    get_filename_component(_basepath ${_relativename} DIRECTORY)
    get_filename_component(_basename ${_relativename} NAME)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_relativename}.h")
    set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_relativename}.cpp")
    if(_basepath)
        set(_moc "${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/${_basename}.moc")
    else()
        set(_moc "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")
    endif()

    get_source_file_property(_nonamespace ${_interface} NO_NAMESPACE)
    if(_nonamespace)
        set(_params -N -m)
    else()
        set(_params -m)
    endif()

    get_source_file_property(_classname ${_interface} CLASSNAME)
    if(_classname)
        set(_params ${_params} -c ${_classname})
    endif()

    get_source_file_property(_include ${_interface} INCLUDE)
    if(_include)
        set(_params ${_params} -i ${_include})
    endif()

    # older dtk didn't set this variable so...
    if(NOT DEFINED DTK_XML2CPP)
        find_program(DTK_XML2CPP "qdbusxml2cpp-fix")
    endif()

    add_custom_command(OUTPUT "${_impl}" "${_header}"
        COMMAND ${DTK_XML2CPP} ${_params} -p ${_relativename} ${_infile}
        DEPENDS ${_infile} ${DTK_XML2CPP}
        VERBATIM
    )

    set_source_files_properties("${_impl}" "${_header}" PROPERTIES
        SKIP_AUTOMOC TRUE
        SKIP_AUTOUIC TRUE
    )

    qt_generate_moc("${_header}" "${_moc}")

    list(APPEND ${_sources} "${_impl}" "${_header}")
    set_property(SOURCE "${_impl}" APPEND PROPERTY OBJECT_DEPENDS "${_moc}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)
endfunction()

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.daemon.Dock1.xml
PROPERTIES
    CLASSNAME Dock1
    NO_NAMESPACE ON
)

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.daemon.Launcher1.xml
PROPERTIES
    CLASSNAME DaemonLauncher1
    NO_NAMESPACE ON
)

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.Appearance1.xml
PROPERTIES
    CLASSNAME Appearance1
    NO_NAMESPACE ON
)

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/xml/org.desktopspec.ApplicationManager1.Application.xml
PROPERTIES
    CLASSNAME AppManager1Application
    NO_NAMESPACE ON
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DBusCPP_SRC "")

dtk_add_dbus_interface(DBusCPP_SRC ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.daemon.Dock1.xml DaemonDock1)
dtk_add_dbus_interface(DBusCPP_SRC ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.daemon.Launcher1.xml DaemonLauncher1)
dtk_add_dbus_interface(DBusCPP_SRC ${CMAKE_CURRENT_LIST_DIR}/xml/org.deepin.dde.Appearance1.xml Appearance1)
dtk_add_dbus_interface(DBusCPP_SRC ${CMAKE_CURRENT_LIST_DIR}/xml/org.desktopspec.ApplicationManager1.Application.xml AppManager1Application)

message(INFO ${DBusCPP_SRC})

add_library(dde-integration-dbus OBJECT
    ${DBusCPP_SRC}
    types/qrect.h

    appwiz.cpp
    ddedock.cpp
    appearance.cpp
    appmgr.cpp
)

target_sources(dde-integration-dbus PUBLIC
FILE_SET HEADERS
FILES
    appwiz.h
    ddedock.h
    appearance.h
    appmgr.h
)

target_include_directories(dde-integration-dbus
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(dde-integration-dbus
PUBLIC
    Qt5::Concurrent
PRIVATE
    Qt5::Core Qt5::Gui Dtk::Core AppStreamQt launcher-utils)
