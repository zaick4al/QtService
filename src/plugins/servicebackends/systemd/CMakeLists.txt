cmake_minimum_required(VERSION 3.5)
project(qsystemd VERSION 1.0 LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")
set(systemd_SRCS
    ${systemd_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/de.skycoder42.QtService.ServicePlugin.systemd.xml
)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

# set_source_files_properties("de.skycoder42.QtService.ServicePlugin.systemd.xml" PROPERTIES CLASSNAME SystemdAdaptor)

qt_add_dbus_interface(MY_VAR "de.skycoder42.QtService.ServicePlugin.systemd.xml" systemd_interface)

message(STATUS ${MY_VAR})

qt_add_dbus_adaptor(MY_VAR2 "de.skycoder42.QtService.ServicePlugin.systemd.xml" servicebackend.h QtService::ServiceBackend SystemdAdaptor)

message(STATUS ${MY_VAR2})

qt_add_plugin(qsystemd CLASS_NAME SystemdServicePlugin SHARED
    systemdservicebackend.cpp systemdservicebackend.h
    systemdservicecontrol.cpp systemdservicecontrol.h
    systemdserviceplugin.cpp systemdserviceplugin.h
    ${MY_VAR}
    ${MY_VAR2}
)

target_include_directories(qsystemd PRIVATE ../../../service
${CMAKE_BINARY_DIR}/QtService/src/plugins/servicebackends/systemd
)

target_link_libraries(qsystemd PRIVATE
    Qt::Core
    Qt::DBus
    Qt6Service
)

target_link_libraries(qsystemd INTERFACE PkgConfig::SYSTEMD)

install(TARGETS qsystemd
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
