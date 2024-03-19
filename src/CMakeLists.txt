cmake_minimum_required(VERSION 3.12)

# Project
project(cuteataxx-gui VERSION 1.0 LANGUAGES CXX)

include_directories(${cuteataxx_SOURCE_DIR}/src/core/)
include_directories(${cuteataxx_SOURCE_DIR}/libs/)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Flags
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -pedantic -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g")


file(COPY ${CMAKE_SOURCE_DIR}/res/piece_images DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/res/board_images DESTINATION ${CMAKE_BINARY_DIR})

qt_standard_project_setup()

qt_add_executable(
    cuteataxx-gui
    
    ./main.cpp
    ./mainwindow.cpp
    ./humanengine.cpp
    ./gameworker.cpp
    ./countdowntimer.cpp
    ./guisettings.cpp
    ./texteditor.cpp

    ./boardview/graphicspiece.cpp
    ./boardview/graphicsboard.cpp
    ./boardview/boardscene.cpp
    ./boardview/boardview.cpp

    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/adjudicate.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/parse_move.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/engine/create.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/play.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/pgn.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/parse/settings.cpp
)

target_link_libraries(
    cuteataxx-gui
    PRIVATE
    Threads::Threads
    Qt6::Widgets
    ataxx_static
    nlohmann_json::nlohmann_json
)
