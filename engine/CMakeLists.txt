cmake_minimum_required(VERSION 3.15)
project(ethbot CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(ethbot
  src/main.cpp
  src/ema.cpp
  src/strategy.cpp
  src/http.cpp
  src/data_client.cpp
  src/order_placer.cpp
)

target_include_directories(ethbot PRIVATE include)
target_link_libraries(ethbot PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)