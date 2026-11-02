cmake_minimum_required(VERSION 3.10)

project(TetrisGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(radtris
    src/Main.cpp
    src/Cell.cpp
    src/Game.cpp
    src/Piece.cpp
    src/Board.cpp
    src/Animations.cpp
)

target_include_directories(radtris PRIVATE
    include
)

if (UNIX AND NOT APPLE)
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)
    target_include_directories(radtris PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(radtris PRIVATE ${CURSES_LIBRARIES})
endif()
