#ifndef GAME_H
#define GAME_H

#include <iostream> //function libraries
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <format>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <ncurses.h>
#endif

using namespace std;

#include "Cell.h" // includes functions and variables from the Cell, Piece, and Board header files
#include "Piece.h"
#include "Board.h"


class Game 
{
private:

    Board board; //board variable
    Piece curr_piece; // current piece variable
    vector<Cell> final_points;
    int linesbuilt = 0; // variable for number of lines built
    int score = 0; // variable for the score
    int speed; // variable for the speed
    int speedCounter = 0; // variable for the 
    int speedCounterincrem = 2; // intital value of the speed incrementor
    int piececount = 0; // number of pieces that have fallen
    int level = 1; // intial value of the level
    bool run; // boolean run variable

    string score_string; //string output for the score
    string line_string;
    string remove_row_string;
    enum class Input{
        up,
        down,
        right,
        left,
        none
    };

    Input readInput();

public:

    Game(); //constructor
    void controls(); // contains the key bindings for the game controls
    void running(); // main game loop, this contains the main game sequence
    void InitConsole();
    void ShutdownConsole();
    void DrawAt(int x, int y, char ch);
    void DrawAt(int x, int y, const char *text);

    bool blocksland(); // checks for collisions with the floor, wall, or with other pieces
    bool checked_move(enum move_direction dir); // checks if the piece is still able to move
    bool checked_rotate(); // checks if the piece is still able to rotate

    void draw(); // draws the board on the screen
    void refreshall(); //refreshes the final points on the screen

    void ClearScreen(); // function that prevents the screen from rapidly flickering and jumping
    bool game_over(); // bool function that tells whether the conditions for a game over are met or not
    bool regame(); // allows for the game to restarts if the player chooses

};


#endif
