cmake_minimum_required(VERSION 3.16)
project(ohmydbms)

add_compile_options(-Wall -O0)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost 1.71.0 COMPONENTS serialization log log_setup REQUIRED)
find_package(unofficial-http-parser REQUIRED)
find_package(fmt 7.1.0 REQUIRED)
find_package(restinio CONFIG REQUIRED)
find_package(json-dto CONFIG REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

file(GLOB MODULE_DATABASE
        "database/*.h"
        "database/*.cpp"
        "database/**/*.h"
        "database/**/*.cpp")

file(GLOB MODULE_TESTS
        "gtest/*.h"
        "gtest/*.cpp"
        "gtest/**/*.h"
        "gtest/**/*.cpp")

file(GLOB MODULE_SYS
        "sys/*.h"
        "sys/*.cpp"
        "sys/**/*.h"
        "sys/**/*.cpp")

file(GLOB MODULE_PARSER
        "parser/*.h"
        "parser/*.cpp"
        "parser/**/*.h"
        "parser/**/*.cpp")

add_subdirectory(gtest)

add_executable(ohmydbms main.cpp helper.cpp helper.h ${MODULE_SYS} ${MODULE_DATABASE} ${MODULE_PARSER} state.cpp state.h Session.cpp Session.h)
target_link_libraries(ohmydbms ${Boost_LIBRARIES} fmt::fmt)

add_executable(ohmydbms_server main_server.cpp helper.cpp helper.h ${MODULE_SYS} ${MODULE_DATABASE} ${MODULE_PARSER} state.cpp state.h Session.cpp Session.h)
target_link_libraries(ohmydbms_server ${Boost_LIBRARIES} fmt::fmt restinio::restinio json-dto::json-dto)