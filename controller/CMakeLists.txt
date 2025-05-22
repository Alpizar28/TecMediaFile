cmake_minimum_required(VERSION 3.15)
project(controller)

add_executable(controller
    src/main.cpp
    src/Controller.cpp      # <-- aquí añadimos tu implementación
)

target_include_directories(controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(controller PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
)
