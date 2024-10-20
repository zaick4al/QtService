@echo on
setlocal

REM Define paths
set QT_PATH=C:\Qt
set QT_TOOLS_PATH=%QT_PATH%\Tools
set QT_TOOLS_CMAKE_PATH=%QT_TOOLS_PATH%\CMake_64\bin
set QT_TOOLS_NINJA_PATH=%QT_TOOLS_PATH%\Ninja
set QT_VERSION=6.6.0
set QT_PREFIX_PATH=%QT_PATH%\6.5.3\mingw_64

REM create destination folders
set BUILD_DIR=build
set INSTALL_DIR="%BUILD_DIR%\install"
set INSTALL_HEADER_DIR="%INSTALL_DIR%\include\QtService"
set INSTALL_LIB_DIR="%INSTALL_DIR%\lib"
set INSTALL_LIB_CMAKE_DIR="%INSTALL_LIB_DIR%\cmake"
set INSTALL_PLUGINS_DIR="%INSTALL_DIR%\plugins"

mkdir "%BUILD_DIR%"
mkdir "%INSTALL_DIR%"
mkdir "%INSTALL_LIB_DIR%"
mkdir "%INSTALL_LIB_CMAKE_DIR%"
mkdir "%INSTALL_PLUGINS_DIR%"

REM copy QtService header files
xcopy /Y "..\src\service\qtservice_global.h" "%INSTALL_HEADER_DIR%\"
xcopy /Y "..\src\service\qtservice_helpertypes.h" "%INSTALL_HEADER_DIR%\"
xcopy /Y "..\src\service\service.h" "%INSTALL_HEADER_DIR%\"
xcopy /Y "..\src\service\CMakeLists.txt" "%INSTALL_LIB_CMAKE_DIR%\"
xcopy /Y "..\src\plugins\" "%INSTALL_PLUGINS_DIR%\" /e /s

REM build QtService
$env:Path = ";C:\Qt\Tools\mingw1120_64\bin\"
$env:Path = ";C:\Qt\Tools\Ninja\"


cd build
cmake -G Ninja ../../
cmake --build .
cd ..

REM copy QtService libraries
REM C:\Project\QtService\build\src\service\Qt6Service.lib
IF NOT EXIST "%BUILD_DIR%\src\service\libQt6Service.a" (
    echo "Error no found libQt6Service.a library"
    exit /b 1
)

echo pwd

xcopy /Y "%BUILD_DIR%\src\service\libQt6Service.a" "%INSTALL_LIB_DIR%\"