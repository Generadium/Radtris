#ifndef GAME_H
#define GAME_H

#include <iostream> //function libraries
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <format>
#include <random>
#include <clocale>
#include <cwchar>
#include <unistd.h>
#include <ncurses.h>

using namespace std;

#include "Cell.h" // includes functions and variables from the Cell, Piece, and Board header files
#include "Piece.h"
#include "Board.h"

class Leaderboard {
public:
    void load();
    void add_score(const std::string& name, int score);
    void save() const;

    struct Score {
    std::string name;
    int score;
    };

    std::vector<Score> top_score(std::size_t limit = 10) const;

private:
    std::vector<Score> entries;
};

class Game 
{
private:

    Board board; //board variable
    Piece piece; // current piece variable
    piece_type next;
    piece_type type;
    vector<Cell> final_points;
    vector<piece_type> bag;

    int lines_built = 0; // variable for number of lines built
    int score = 0; //score set to 0
    int piece_count = 0; // number of pieces that have fallen
    int level = 0; // intial value of the level
    int screen_width = 0;
    int screen_height = 0;

    bool run; // boolean run variable
    bool paused = false;
    bool piece_active = false;
    bool lock_active = false;
    bool slam_request = false;

    std::mt19937 randomEngine{std::random_device{}()};

    using Clock = std::chrono::steady_clock;

    Clock::time_point last_fall_time;
    Clock::time_point grounded_since;

    std::chrono::milliseconds grav_interval{500};
    std::chrono::milliseconds lock_delay{500};

    struct Controls {
        const char* key;
        const char* action;
    };

    static constexpr Controls controls[] = {
        {"<- / ->", "Move left/right"},
        {"^", "Rotate"},
        {"v", "Drop"},
        {"Space", "Slam"},
        {"p", "Pause game"}
    };

    enum class Input{
        up,
        down,
        right,
        left,
        space,
        pause,
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
    Result readResult(Input input);

public:

    Game(); //constructor

    void running(); // main game loop, this contains the main game sequence
    void draw_cell(int x, int y, char ch);
    void draw_cell(int x, int y, const char *text);
    void draw_cell(int x, int y, const wchar_t* text);
    void draw(bool present = true); // draws the board on the screen
    void refreshall(); //refreshes the final points on the screen
    void lock(); //converts the piece to built cells
    void update_lock_delay(Result result, Clock::time_point now);
    void refill();
    void animate_line(const std::vector<int>& rows);
    void animate_board_init();
    void animate_line_wipes(bool game_over, bool regame);
    void animate_board_erase();
    void refresh_board();
    void draw_clear_frame(const std::vector<int>& rows, int dissolvedDistance, bool flashOn);
    void white(int x, int y);
    
    bool blocksland(); // checks for collisions with the floor, wall, or with other pieces
    bool checked_move(enum move_direction dir); // checks if the piece is still able to move
    bool checked_rotate(); // checks if the piece is still able to rotate
    bool can_fall(); // checks if the piece is able to fall
    bool checked_fall(); //checks if the piece is still falling
    bool game_over(); // bool function that tells whether the conditions for a game over are met or not
    bool regame(); // allows for the game to restarts if the player chooses

    int score_calc(const std::vector<int> completed_rows, int level);

    piece_type generate_piece_type();

};
#endif