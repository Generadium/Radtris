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
    int score = 0; //score set to 0
    //phase out
    int speed = 20; //speed set to 20 ms
    int speedCounter = 0; // variable for the 
    int speedCounterincrem = 2; // intital value of the speed incrementor

    int piececount = 0; // number of pieces that have fallen
    int level = 1; // intial value of the level
    bool run; // boolean run variable
    bool piece_active = false;
    bool lock_active = false;
    bool slam_request = false;

    using Clock = std::chrono::steady_clock;

    Clock::time_point last_fall_time;
    Clock::time_point grounded_since;

    std::chrono::milliseconds grav_interval{500};
    std::chrono::milliseconds lock_delay{500};

    string score_string; //string output for the score
    string line_string; //string output for lines built

    enum class Input{
        up,
        down,
        right,
        left,
        space,
        none
    };

    enum class Result{
        none,
        move,
        rotate,
        soft_drop,
        slam
    };

    Input readInput();
    Result readResult();;

public:

    Game(); //constructor
    void controls(); // contains the key bindings for the game controls
    void running(); // main game loop, this contains the main game sequence
    void InitConsole();
    void ShutdownConsole();
    void DrawAt(int x, int y, char ch);
    void DrawAt(int x, int y, const char *text);

    bool blocksland(); // checks for collisions with the floor, wall, or with other pieces
    //REWORK
    bool checked_move(enum move_direction dir); // checks if the piece is still able to move
    bool checked_rotate(); // checks if the piece is still able to rotate
    bool can_fall(); // checks if the piece is able to fall
    bool checked_fall(); //checks if the piece is still falling
    bool is_grounded();

    void draw(); // draws the board on the screen
    void refreshall(); //refreshes the final points on the screen
    void lock(); //converts the piece to built cells
    void slam();
    void update_lock_delay(Result result, Clock::time_point now);

    void ClearScreen(); // function that prevents the screen from rapidly flickering and jumping
    bool game_over(); // bool function that tells whether the conditions for a game over are met or not
    bool regame(); // allows for the game to restarts if the player chooses

};

#endif