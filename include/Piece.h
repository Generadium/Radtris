#ifndef PIECE_H
#define PIECE_H

#include "Cell.h" //header files and function libraries
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

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
    right = -2,
    left = 2,
};

class Piece {
private:
    enum piece_type type; //type variable for the pieces shapes
    vector<Cell> body; // body variable for the piece shapes
    Cell pos; // variable for the position of the blocks

public:
    Piece() = default; //default constructor
    Piece(Cell pos, piece_type type); // constructor that uses the pos variable
    void fall_down(); //function that causes pieces to automatically fall down
    vector<Cell> fallen_body(); // function that creates the body of the piece after it has fallen
    void move(enum move_direction dir); // function that causes the pieces to move left or right
    vector<Cell> moved_body(enum move_direction dir); // function that creates the body of the piece after it has moved
    void rotate(); //function that causes pieces to rotate
    vector<Cell> rotated_body(); // function that creates the body of the piece that has rotated
    vector<Cell> get_body() 
    { 
        return body; 
    } //mutator that accesses the enum piece body variable
    void set_body(vector<Cell> body) 
    { 
        this->body = body;
    } //points to the address of body

    void build_body(piece_type type);

};

#endif
