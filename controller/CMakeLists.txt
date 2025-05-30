cmake_minimum_required(VERSION 3.16)
project(controller)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pthreads
find_package(Threads REQUIRED)

# JSON (si lo usas)
find_package(nlohmann_json CONFIG REQUIRED)

# 1) Ejecutable con tus fuentes
add_executable(controller
    src/main.cpp
    src/FileController.cpp

    # Añadimos también las fuentes de shared para que se compile Raid5, Base64 y JsonProtocol
    ${CMAKE_SOURCE_DIR}/shared/src/Raid5.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/Base64.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/JsonProtocol.cpp
    ${CMAKE_SOURCE_DIR}/shared/src/Raid5Helpers.cpp

)

# 2) Directorios de headers
target_include_directories(controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include         # FileController.h :contentReference[oaicite:0]{index=0}
    ${CMAKE_SOURCE_DIR}/shared/include          # Raid5.h, Base64.h, JsonProtocol.h
    ${CMAKE_BINARY_DIR}/_deps/crow-src/include  # Crow
    ${CMAKE_BINARY_DIR}/_deps/json-src/include  # nlohmann_json (si no usas find_package)
)

# 3) Linkeo de librerías
target_link_libraries(controller PRIVATE
    Crow
    Threads::Threads
    nlohmann_json::nlohmann_json
    shared
)
