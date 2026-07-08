#include "Cell.h"
#include "Piece.h"
#include "Board.h"
#include "Game.h"


bool titleScreen();
int main() // main function, contains everything
{
    if (titleScreen() == true) // if the titleScreen function is true, then the game starts
    {
        Game game; // calls the constructor in the game file
    }
    else // otherwise, the program will terminate
        return 0;
}
bool titleScreen() // main menu command
{
        char c{};
        cout << "\n              TETRIS                       \n\n\n\n\n";
        cout << "        Enter 'y' to play                  \n";
        cout << "        or any other character to close      ";
        cin >> c;

        if (c == 'y')
            return true;
        else
            return false;
        

       
}