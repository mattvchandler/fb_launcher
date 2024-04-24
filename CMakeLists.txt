cmake_minimum_required (VERSION 3.14.2...3.18)
project(fb_launcher VERSION 1.0.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Framebuffer app launcher")
set(AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(PNG REQUIRED)
find_package(Fontconfig REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    sdl.cpp
    font.cpp)

target_link_libraries(${PROJECT_NAME}
    SDL2::Main
    SDL2::TTF
    PNG::PNG
    Fontconfig::Fontconfig)
