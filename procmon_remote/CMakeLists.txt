cmake_minimum_required(VERSION 3.10)
project(procmon_remote)

set(CMAKE_CXX_STANDARD 11)
# remove any UNICODE/_UNICODE definitions or undefine them:
add_definitions(-UUNICODE -U_UNICODE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ensure the build dir (where uic puts ui_*.h) is on your include path
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Set the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "F:/VCPKG/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the CMAKE_PREFIX_PATH to the vcpkg installed directory
set(CMAKE_PREFIX_PATH "F:/VCPKG/vcpkg/installed/x64-windows;F:/Qt3/6.9.0/mingw_64/lib/cmake/Qt6")

set(QT_BIN_DIR "F:/Qt3/6.9.0/mingw_64/bin")

# Find Qt6 libraries
find_package(Qt6 COMPONENTS Core Gui Widgets Charts REQUIRED)

# Find the libssh package
find_package(libssh CONFIG REQUIRED)

add_executable(procmon_remote main.cpp ProcessInfo.cpp
        ServerClient.hpp
        ServerClient.cpp
        RaspberryProcesses.cpp
        RaspberryProcesses.hpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        HandleDLLInspection.cpp
        HandleDLLInspection.hpp
        FileRegistryTrace.cpp
        FileRegistryTrace.hpp
)

# Link the libssh library
target_link_libraries(procmon_remote PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts ssh ws2_32 Advapi32 Crypt32 psapi pdh)

# if on Windows (32- or 64-bit), always deploy Qt and libssh DLLs
if(WIN64 OR WIN32)
# 1) Deploy Qt6 DLLs
add_custom_command(TARGET procmon_remote POST_BUILD
        COMMAND
        "${Qt6_DIR}/../../../bin/windeployqt.exe"
        --qmldir "${CMAKE_SOURCE_DIR}"
        "$<TARGET_FILE:procmon_remote>"
)
# 2) Copy libssh DLL
add_custom_command(TARGET procmon_remote POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ssh>
        "$<TARGET_FILE_DIR:procmon_remote>/libssh-4.dll"
)
endif()

