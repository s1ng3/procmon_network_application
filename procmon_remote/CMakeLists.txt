cmake_minimum_required(VERSION 3.10)
project(procmon_remote)

set(CMAKE_CXX_STANDARD 11)

add_executable(procmon_remote main.cpp ProcessInfo.cpp
        ServerClient.hpp
        ServerClient.cpp
)

target_link_libraries(procmon_remote ws2_32)