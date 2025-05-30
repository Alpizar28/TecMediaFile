cmake_minimum_required(VERSION 3.15)
project(shared)

add_library(shared STATIC
    src/Raid5.cpp
    src/JsonProtocol.cpp
    src/Base64.cpp
    src/Raid5Helpers.cpp
)

target_include_directories(shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
