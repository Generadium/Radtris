#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <functional>
#include <ncurses.h>

using namespace std;

class Board 
{
private:
    int width; // width of the board
    int height; // height of the board
    vector<Cell> all_points; // vector includes all points on the board
    vector<Cell> built_points; // vector includes all points occupied by blocks
public:
    Board(int width = 24, int height = 22); // constructor sets board to height and width

    int get_width() 
    { return width; } // mutator for width
    int get_height() 
    { return height; } // mutator for height
    
    vector<Cell> get_all_points()
    { return all_points; } // mutator for all points vector
    vector<Cell> get_built_points() 
    { return built_points; } // mutator for built points variable 
    void set_built_points(vector<Cell> built_points) 
    { this->built_points = built_points; } // built points accessor, uses this pointer
    void set_border(); // accessor for border
    

    void insert_to_built_points(vector<Cell> insert_points);
    int remove_row(); // removes row of tiles when row is filled
    void refresh(); // refreshes the board in the game loop

};

#endif
