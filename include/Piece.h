#ifndef PIECE_H
#define PIECE_H

#include "Cell.h" //header files and function libraries
#include <vector>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <ncurses.h>
#endif

using namespace std;

enum piece_type  // enum variables for the piece shapes
{
    t_piece,
    i_piece,
    o_piece,
    l_piece,
    j_piece,
    s_piece,
    z_piece
};
 
enum move_direction // enum variables for the movement of the blocks
{
    right = 1,
    left = -1,
};

class Piece {
private:
    enum piece_type type; //type variable for the pieces shapes
    vector<Cell> body; // body variable for the piece shapes
    Cell pos; // variable for the position of the blocks


public:
    Piece() = default; //default constructor
    Piece(Cell pos); // constructor that uses the pos variable
    void fall_down(); //function that causes pieces to automatically fall down
    vector<Cell> fallenbody(); // function that creates the body of the piece after it has fallen
    void move(enum move_direction dir); // function that causes the pieces to move left or right
    vector<Cell> movedbody(enum move_direction dir); // function that creates the body of the piece after it has moved
    void rotate(); //function that causes pieces to rotate
    vector<Cell> rotatedbody(); // function that creates the body of the piece that has rotated
    vector<Cell> get_body() 
    { 
        return body; 
    } //mutator that accesses the enum piece body variable
    void set_body(vector<Cell> body) 
    { 
        this->body = body;
    } //points to the address of body

};

#endif
