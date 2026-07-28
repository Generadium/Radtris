#include "Board.h"

Board::Board(int width, int height) //constructor
    :width{ width }, height{ height }
{
    for (int i{ 0 }; i < width; i++)
        for (int j{ 0 }; j < height; j++)
            all_points.push_back(Cell(i, j));

    set_border();
}

void Board::refresh() // sets all recently fallen blocks or tiles to new built tiles
{ //refreshes the board
    set_border();
    for (auto& all_pnt : all_points) // auto pointer that points to all tiles
        for (auto built_pnt : built_points) //sets all built points in place
        {
            if (all_pnt == built_pnt)
            {
                all_pnt = built_pnt;  //seting the char of all_pnt to built pnt
            }
        }
}

void Board::insert_to_built_points(vector<Cell> insert_points) 
{built_points.insert(built_points.end(), insert_points.begin(), insert_points.end());}

void Board::set_border()
{
    for (auto& point : all_points)
    {
        int x = point.get_x();
        int y = point.get_y();

        if (y == height - 1)
        {
            if (x == 0 || x == width - 1)
                point.set_type(' ');
            else if (x % 2 == 0)
                point.set_type('/');
            else
                point.set_type('\\');
        }
        else if (y == height - 2)
        {
            if (x == 0)
                point.set_type('<');
            else if (x == width - 1)
                point.set_type('>');
            else
                point.set_type('=');
        }
        else if (x == 0)
            point.set_type('<');
        else if (x == width - 1)
            point.set_type('>');
        else if (x == 1 || x == width - 2)
            point.set_type('!');
        else if (x % 2 != 0)
            point.set_type('.');
        else 
            point.set_type(' ');
    }
}

//edit to configure animations/remove_rows
void Board::remove_row(const std::vector<int>& rows)
{
    built_points.erase(
        std::remove_if(
            built_points.begin(),
            built_points.end(),
            [&rows](const Cell& point) {
                return std::find(
                    rows.begin(),
                    rows.end(),
                    point.get_y()
                ) != rows.end();
            }
        ),
        built_points.end()
    );

    for (auto& point : built_points) {
        const int clearedBelow = std::count_if(
            rows.begin(),
            rows.end(),
            [&point](int row) {
                return row > point.get_y();
            }
        );

        point.move(0, clearedBelow);
    }
}

std::vector<int> Board::get_completed_rows() const
{
    std::vector<int> rows;

    for (int row = 1; row < height - 2; ++row) {
        const int count = std::count_if(
            built_points.begin(),
            built_points.end(),
            [row](const Cell& point) {
                return point.get_y() == row;
            }
        );

        if (count == (width / 2) - 2)
            rows.push_back(row);
    }

    return rows;
}

void Board::clear_built_points()
{
    built_points.clear();
    set_border();
}