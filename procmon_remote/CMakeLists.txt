cmake_minimum_required(VERSION 3.10)
project(procmon_remote)

set(CMAKE_CXX_STANDARD 11)

# Add the path to your vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "F:/VCPKG/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the CMAKE_PREFIX_PATH to the vcpkg installed directory
set(CMAKE_PREFIX_PATH "F:/VCPKG/vcpkg/installed/x64-windows")

# Find the libssh package
find_package(libssh CONFIG REQUIRED)

add_executable(procmon_remote main.cpp ProcessInfo.cpp
        ServerClient.hpp
        ServerClient.cpp
        RaspberryProcesses.cpp
        RaspberryProcesses.hpp
)

# Link the libssh library
target_link_libraries(procmon_remote PRIVATE ssh ws2_32)