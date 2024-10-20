cmake_minimum_required(VERSION 3.16)
project(qstandard VERSION 1.0 LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_add_plugin(qstandard CLASS_NAME StandardServicePlugin SHARED

    standardservicebackend.cpp standardservicebackend.h
    standardservicecontrol.cpp standardservicecontrol.h
    standardserviceplugin.cpp standardserviceplugin.h
)
target_include_directories(qstandard PRIVATE ../../../service)
target_link_libraries(qstandard PRIVATE
    Qt::Core
    Qt6Service
)

if(WIN32)
    target_link_libraries(qstandard PUBLIC
        kernel32
    )
endif()

install(TARGETS qstandard
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
