cmake_minimum_required(VERSION 3.15)
project(disk)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(disknode
    src/main.cpp
    src/DiskNode.cpp
)

target_include_directories(disknode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(disknode PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
)
