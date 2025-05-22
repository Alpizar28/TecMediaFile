cmake_minimum_required(VERSION 3.15)
project(TECMFS_Crow)

include(FetchContent)

# Crow
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(crow)

# nlohmann_json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# Catch2
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(catch2)

add_subdirectory(shared)
add_subdirectory(controller)
add_subdirectory(disk)
add_subdirectory(tests)
