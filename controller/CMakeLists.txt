# controller/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(controller)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pthreads
find_package(Threads REQUIRED)

# JSON (si lo usas)
find_package(nlohmann_json CONFIG REQUIRED)

# 1) Definir el ejecutable
add_executable(controller
    src/main.cpp
    src/FileController.cpp
)

# 2) Directorios de headers
target_include_directories(controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include                  # tu FileController.h
    ${CMAKE_BINARY_DIR}/_deps/crow-src/include           # headers de Crow
    ${CMAKE_BINARY_DIR}/_deps/json-src/include           # headers de nlohmann_json (si no usas find_package)
)

# 3) Linkeo de librerías
target_link_libraries(controller PRIVATE
    Crow
    Threads::Threads
    nlohmann_json::nlohmann_json
)
