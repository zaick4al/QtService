cmake_minimum_required(VERSION 3.16)

project(Qt6ServiceProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network) # special case

set(HEADER_FILES
    terminalserver_p.h
    terminalclient_p.h
    serviceplugin.h
    servicebackend.h
    servicecontrol.h
)

set(SOURCE_FILES
    service.h
    service.cpp
    servicebackend.cpp
    servicecontrol.cpp
    terminal.h
    terminal.cpp
    terminalserver.cpp
    terminalclient.cpp
    serviceplugin.cpp
)

qt_add_library(Qt6Service SHARED ${SOURCE_FILES} ${HEADER_FILES})

# target_compile_definitions(Qt6Service PUBLIC QT_STATIC)

target_include_directories(Qt6Service PRIVATE ../../libs/QConsole ../../libs/QCtrlSignals/src)

target_link_libraries(Qt6Service PRIVATE Qt6::Core Qt6::CorePrivate Qt6::Network QConsole QCtrlSignals)

target_compile_definitions(Qt6Service PRIVATE QT_BUILD_SERVICE_LIB= Q_CTRL_SIGNALS_EXPORT= Q_CONSOLE_EXPORT=)
