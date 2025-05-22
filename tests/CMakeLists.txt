cmake_minimum_required(VERSION 3.15)
project(tests)

enable_testing()

# Catch2 ya está disponible tras FetchContent_MakeAvailable(catch2)
# El target es Catch2::Catch2WithMain

add_executable(controller_tests
    controller/ControllerTests.cpp
)
target_link_libraries(controller_tests PRIVATE
    Catch2::Catch2WithMain
    shared
    Crow::Crow                # <-- corregido aquí
    nlohmann_json::nlohmann_json
)
add_test(NAME controller_tests COMMAND controller_tests)

add_executable(disk_tests
    disk/DiskNodeTests.cpp
)
target_link_libraries(disk_tests PRIVATE
    Catch2::Catch2WithMain
    shared
    nlohmann_json::nlohmann_json
)
add_test(NAME disk_tests COMMAND disk_tests)
