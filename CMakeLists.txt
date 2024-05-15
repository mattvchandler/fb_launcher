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
find_package(PkgConfig REQUIRED)

pkg_check_modules(CEC libcec REQUIRED)
pkg_check_modules(SVG librsvg-2.0 REQUIRED)

add_subdirectory(csvpp)

add_executable(${PROJECT_NAME}
    main.cpp
    app.cpp
    cec.cpp
    font.cpp
    joystick.cpp
    menu.cpp
    texture.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CEC_INCLUDE_DIRS}
    PRIVATE ${SVG_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    SDL2::Main
    SDL2::TTF
    PNG::PNG
    Fontconfig::Fontconfig
    csvpp::csvpp
    ${CEC_LIBRARIES}
    ${SVG_LIBRARIES}
)

function(embed_and_link_files EXECUTABLE_NAME SOURCE_FILES)
    foreach(FILE ${SOURCE_FILES})
        get_filename_component(FILE_NAME ${FILE} NAME)
        get_filename_component(FILE_DIR ${FILE} DIRECTORY)
        set(OUTPUT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}.o")

        add_custom_command(
            OUTPUT ${OUTPUT_OBJECT}
            COMMAND ld -b binary -r -z noexecstack "${FILE_NAME}" -o "${OUTPUT_OBJECT}"
            DEPENDS ${FILE}
            WORKING_DIRECTORY "${FILE_DIR}"
            COMMENT "Embedding ${FILE_NAME}"
        )

        target_sources(${EXECUTABLE_NAME} PRIVATE ${OUTPUT_OBJECT})
    endforeach()
endfunction()

set(EMBEDDED_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/input_icons/computer-mouse.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/input_icons/keyboard.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/input_icons/gamepad.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/input_icons/mobile-retro.svg"
)
embed_and_link_files(${PROJECT_NAME} "${EMBEDDED_FILES}")
