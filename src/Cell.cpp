#include "Cell.h"


Cell::Cell(int x, int y, char type) // default constructor
    :x{ x }, y{ y }, type{ type } {}

bool Cell::operator==(const Cell& rhs) const  // sets x and y to the right hand side of the equals sign in the get functions
{     
    return (x == rhs.get_x() && y == rhs.get_y()); //right hand side
}

Cell Cell::shift_copy(int x_shift, int y_shift) const 
{
    return Cell(x + x_shift, y + y_shift, type); //returns redefined constructor of the tiles allowing them to shift in functions later on
}

void Cell::move(int x_move, int y_move) // allows cells to move in x or y directions
{
    x += x_move;
    y += y_move;
}

void Cell::set_coordinate(int x_new, int y_new) //accessor for the coordinates
{
    x = x_new;
    y = y_new;
}