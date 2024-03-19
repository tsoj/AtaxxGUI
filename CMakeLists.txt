cmake_minimum_required(VERSION 3.16)


project(
    AtaxxGUI
    VERSION 0.0.1
    DESCRIPTION "A GUI for playing Ataxx"
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
    libataxx
    GIT_REPOSITORY https://github.com/kz04px/libataxx
    GIT_TAG 2fe5d1517c9e66fefb78cef7b3c35e5f9f1bb9ee
)

FetchContent_Declare(
    cuteataxx
    GIT_REPOSITORY https://github.com/tsoj/cuteataxx
    GIT_TAG master
)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG v2.4.10
)

FetchContent_MakeAvailable(libataxx json doctest)

FetchContent_GetProperties(cuteataxx)
if(NOT cuteataxx_POPULATED)
  FetchContent_Populate(cuteataxx)
endif()


find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

if(NOT (Boost_FOUND AND Threads_FOUND AND Qt6Core_FOUND))
    message(FATAL_ERROR "Can't build AtaxxGUI: Boost, Threads, and Qt6 required")
endif()

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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

qt_standard_project_setup()

qt_add_executable(
    AtaxxGUI
    
    src/main.cpp
    src/mainwindow.cpp
    src/humanengine.cpp
    src/gameworker.cpp
    src/countdowntimer.cpp
    src/guisettings.cpp
    src/texteditor.cpp

    src/boardview/graphicspiece.cpp
    src/boardview/graphicsboard.cpp
    src/boardview/boardscene.cpp
    src/boardview/boardview.cpp

    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/adjudicate.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/parse_move.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/engine/create.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/play.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/pgn.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/parse/settings.cpp
)

target_link_libraries(
    AtaxxGUI
    PRIVATE
    Threads::Threads
    Qt6::Widgets
    ataxx_static
    nlohmann_json::nlohmann_json
)