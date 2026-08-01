This is an improved version of a Tetris clone that I used as part of
a college project. 

Compile with animations: g++ -std=c++17 -D_XOPEN_SOURCE_EXTENDED   -Iinclude src/*.cpp -o tetris -lncursesw

Compile without animations: g++ src/*.cpp -Iinclude -std=c++17 -lncurses -o tetris
