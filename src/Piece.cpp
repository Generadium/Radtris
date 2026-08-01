#include "Piece.h"
using namespace std;

Piece::Piece(Cell pos, piece_type type) //defines all the pieces on an x,y grid
    :pos{pos}, type{type}
    {
        build_body(type);
    }

void Piece::build_body(piece_type type)
{
    switch (type)
    {
        case t_piece:
            body = { pos, pos.shift_copy(-2, 0), pos.shift_copy(2, 0), pos.shift_copy(0, 1) };
            break;
        case i_piece:
            body = { pos, pos.shift_copy(-2, 0), pos.shift_copy(2, 0), pos.shift_copy(4, 0) };
            break;
        case o_piece:
            body = { pos, pos.shift_copy(2, 0), pos.shift_copy(0, 1), pos.shift_copy(2, 1) };
            break;
        case l_piece:
            body = { pos, pos.shift_copy(0, 1), pos.shift_copy(0, 2), pos.shift_copy(2, 0) };
            break;
        case j_piece:
            body = { pos, pos.shift_copy(-2, 0), pos.shift_copy(0, 1), pos.shift_copy(2, 1) };
            break;
        case s_piece:
            body = { pos, pos.shift_copy(-2, 0), pos.shift_copy(0, 1), pos.shift_copy(2, 1) };
            break;
        case z_piece:
            body = { pos, pos.shift_copy(-2, 1), pos.shift_copy(0, 1), pos.shift_copy(2, 0) };
            break;
    }
}

void Piece::fall_down() // automatically causes pieces to fall
{
    pos.move(0, +1); //allows piece to continue falling while rotating
    for (auto& point : body) // creates a pointer that points to the address of 'body'
        point.move(0, +1); //move the pieces down automatically -1 y positions during each iteration
}

vector<Cell> Piece::fallen_body() // allows the piece's body to shift downward by creating a copy of it at -1y increments
{
    vector<Cell> next_body;
    for (const auto& point : body) // creates a pointer that does not modify body
        next_body.push_back(point.shift_copy(0, +1)); // uses the push back command and a pointer to point to the -1 y position

    return next_body;
}

void Piece::move(enum move_direction dir) //moves the pieces left or right
{
    pos.move(dir, 0);
    for (auto& point : body)
        point.move(dir, 0);
}

vector<Cell> Piece::moved_body(enum move_direction dir)  // creates the body of the piece after it has been moved
{
    vector<Cell> next_body;
    for (const auto& point : body) // pointer to body
        next_body.push_back(point.shift_copy(dir, 0)); // shifts a copy of the body to the direction of the movement
    return next_body;
}

void Piece::rotate() //rotates the pieces
{
    for (auto& point : body) 
    {
        int point_x = (point.get_x() - pos.get_x()) / 2;
        int point_y = point.get_y() - pos.get_y();
        point.set_coordinate((-point_y * 2) + pos.get_x(), point_x + pos.get_y());
    }
}

vector<Cell> Piece::rotated_body() //creates the rotated body of the piece
{
    vector<Cell> next_body;
    for (const auto& point : body)
    {
        int point_x = (point.get_x() - pos.get_x())/2;
        int point_y = point.get_y() - pos.get_y();
        next_body.push_back(Cell((-point_y * 2) + pos.get_x(), point_x + pos.get_y(),
        point.get_type()));
    }
    return next_body;
}