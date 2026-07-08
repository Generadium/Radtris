cmake_minimum_required(VERSION 3.10)

project(TetrisGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tetris
    src/Main.cpp
    src/Cell.cpp
    src/Game.cpp
    src/Piece.cpp
    src/Board.cpp
)

target_include_directories(tetris PRIVATE
    include
)

if (UNIX AND NOT APPLE)
    find_package(Curses REQUIRED)
    target_include_directories(tetris PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(tetris PRIVATE ${CURSES_LIBRARIES})
endif()