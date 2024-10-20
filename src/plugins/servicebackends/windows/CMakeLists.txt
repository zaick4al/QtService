cmake_minimum_required(VERSION 3.5)
project(qwindows VERSION 1.0 LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_add_plugin(qwindows CLASS_NAME WindowsServicePlugin SHARED
    windowsservicebackend.cpp windowsservicebackend.h
    windowsservicecontrol.cpp windowsservicecontrol.h
    windowsserviceplugin.cpp windowsserviceplugin.h
)
target_include_directories(qwindows PRIVATE ../../../service)

target_link_libraries(qwindows PRIVATE
    Qt::Core
    advapi32
    Qt6Service
)

install(TARGETS qwindows
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
