#ifndef CELL_H
#define CELL_H

#include <iostream>

class Cell 
{
private:
    int x;
    int y;
    char type;
public:
    Cell(int x = 0, int y = 0, char type = ' '); //default constructor

    int get_x() const 
    { 
        return x; 
    } // const, read only function mutator that allows the program to modify the x variable
    int get_y() const
    {
        return y; 
    } // const, read only function mutator that allows the program to modify the y variable
    char get_type() const 
    { 
        return type; 
    } // const, read only function mutator that allows the program to modify the type
    void set_type(char c) 
    { 
        type = c; 
    } // accessor, sets the type to a parameter


    Cell shift_copy(int x_shift, int y_shift) const; //shifts copies of the char variable to different x and y position
    void move(int x_move, int y_move); //contains the move variables for the x and y variables
    void set_coordinate(int x_new, int y_new); // sets coordinates for the x and y variables

    bool operator==(const Cell& rhs) const; // sets x and y to the right hand side of the equals sign in the mutators
};

#endif