cmake_minimum_required(VERSION 3.16)

project(QtService)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/QConsole)
add_subdirectory(libs/QCtrlSignals)
if (WIN32)
    add_subdirectory(src/plugins/servicebackends/windows)
endif ()
if (UNIX AND NOT APPLE)
    add_subdirectory(src/plugins/servicebackends/systemd)
endif ()
add_subdirectory(src/plugins/servicebackends/standard)
add_subdirectory(src/service)
