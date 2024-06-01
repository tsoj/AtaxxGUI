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
    GIT_REPOSITORY https://github.com/tsoj/libataxx
    GIT_TAG 19046ba6aed31ddce45d26fe0bcabfd72fad5c92
)

FetchContent_Declare(
    cuteataxx
    GIT_REPOSITORY https://github.com/tsoj/cuteataxx
    GIT_TAG a4f381b54a1e14af4524b42e8b7e5a1834d53d30
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


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(Boost REQUIRED)
else()
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost REQUIRED COMPONENTS filesystem)
endif()
find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

if(NOT (Boost_FOUND AND Threads_FOUND AND Qt6Core_FOUND))
    message(FATAL_ERROR "Can't build AtaxxGUI: Boost, Threads, and Qt6 required")
endif()

include_directories(${cuteataxx_SOURCE_DIR}/src/core/)
include_directories(${cuteataxx_SOURCE_DIR}/libs/)
include_directories(${Boost_INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Flags
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -pedantic -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -fexceptions")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

qt_standard_project_setup()

qt_add_executable(
    AtaxxGUI
    
    src/main.cpp
    src/mainwindow.cpp
    src/countdowntimer.cpp

    src/boardview/graphicspiece.cpp
    src/boardview/graphicsboard.cpp
    src/boardview/boardscene.cpp
    src/boardview/boardview.cpp

    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/adjudicate.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/ataxx/parse_move.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/engine/create.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/play.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/pgn.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/parse/openings.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/parse/settings.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/match/run.cpp
    ${cuteataxx_SOURCE_DIR}/src/core/match/worker.cpp
)

target_link_libraries(
    AtaxxGUI
    PRIVATE
    Threads::Threads
    Qt6::Widgets
    Qt6::Charts
    ataxx_static
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
)

set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin")

set_target_properties(AtaxxGUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/$<0:>)
set_target_properties(AtaxxGUI PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}/$<0:>)

file(COPY ${CMAKE_SOURCE_DIR}/res/piece_images DESTINATION ${BIN_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/res/board_images DESTINATION ${BIN_DIR})
