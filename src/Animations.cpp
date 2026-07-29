#include "Game.h"
#include "Board.h"

void Game::draw_clear_frame(
    const std::vector<int>& rows,
    int dissolvedDistance,
    bool flashOn)
{
    // Draw the ordinary board without presenting it yet.
    draw(false);

    int screenHeight;
    int screenWidth;

    getmaxyx(stdscr, screenHeight, screenWidth);

    const int boardX =
        std::max(0, (screenWidth - board.get_width()) / 2);

    const int boardY =
        std::max(0, (screenHeight - board.get_height()) / 2);

    const int blockCount = (board.get_width() / 2) - 2;

    const int centerLeft = blockCount / 2 - 1;
    const int centerRight = blockCount / 2;

    for (int row : rows) {
        for (int block = 0; block < blockCount; ++block) {
            const int distanceFromCenter = std::min(
                std::abs(block - centerLeft),
                std::abs(block - centerRight)
            );

            const bool dissolved =
                distanceFromCenter <= dissolvedDistance;

            // Each logical block occupies two terminal columns.
            const int cellX = 2 + block * 2;

            if (dissolved || !flashOn) {
                draw_cell(
                    boardX + cellX,
                    boardY + row,
                    " ."
                );
            } else {
                white(
                    boardX + cellX,
                    boardY + row
                );
            }
        }
    }

    refresh();
}

void Game::white(int x, int y)
{
    attron(A_REVERSE | A_BOLD);
    mvaddstr(y, x, "  ");
    attroff(A_REVERSE | A_BOLD);
}

void Game::animate_line(const std::vector<int>& rows)
{
    const int blockCount = (board.get_width() / 2) - 2;
    const int dissolveSteps = (blockCount + 1) / 2;

    constexpr auto flashDelay =
        std::chrono::milliseconds(30);

    // Flash the complete rows.
    for (int blink = 0; blink < 3; ++blink) {
        draw_clear_frame(rows, -1, true);
        std::this_thread::sleep_for(flashDelay);

        draw_clear_frame(rows, -1, false);
        std::this_thread::sleep_for(flashDelay);
    }

    // Dissolve outward from the two center blocks.
    for (int distance = 0;
         distance < dissolveSteps;
         ++distance)
    {
        draw_clear_frame(rows, distance, true);
        std::this_thread::sleep_for(flashDelay);

        draw_clear_frame(rows, distance, false);
        std::this_thread::sleep_for(flashDelay);
    }

    // Keep the rows visibly empty.
    draw_clear_frame(rows, dissolveSteps, false);

    // Suspend cells above before removing/collapsing the rows.
    std::this_thread::sleep_for(
        std::chrono::milliseconds(300)
    );
}

void Game::animate_board_init()
{
    int screenHeight;
    int screenWidth;


    getmaxyx(stdscr, screenHeight, screenWidth);
    erase();

    const int boardX =
        std::max(0, (screenWidth - board.get_width()) / 2);

    const int boardY =
        std::max(0, (screenHeight - board.get_height()) / 2);

    const int width = board.get_width();
    const int height = board.get_height();

    constexpr auto wallDelay = std::chrono::milliseconds(25);
    constexpr auto floorDelay = std::chrono::milliseconds(35);
    constexpr auto fillDelay = std::chrono::milliseconds(25);

    // Present everything currently drawn.
    const auto present = []() {refresh();};

    // 1. Draw the vertical walls from top to bottom.
    //
    // Board columns:
    // x == 0         contains '<'
    // x == 1         contains '!'
    // x == width - 2 contains '!'
    // x == width - 1 contains '>'
    for (int y = 0; y < height - 2; ++y) {
        draw_cell(boardX,             boardY + y, '<');
        draw_cell(boardX + 1,         boardY + y, '!');
        draw_cell(boardX + width - 2, boardY + y, '!');
        draw_cell(boardX + width - 1, boardY + y, '>');

        present();
        std::this_thread::sleep_for(wallDelay);
    }

    // Draw the row directly above the patterned bottom border.
    draw_cell(boardX, boardY + height - 2, '<');

    for (int x = 1; x < width - 1; ++x)
        draw_cell(boardX + x, boardY + height - 2, '=');

    draw_cell(
        boardX + width - 1,
        boardY + height - 2,
        '>'
    );

    present();

    // 2. Draw / and \ from the center toward both edges.
    const int bottomY = boardY + height - 1;
    const int centerLeft = (width - 1) / 2;
    const int centerRight = width / 2;

    for (int distance = 0; ; ++distance) {
        const int left = centerLeft - distance;
        const int right = centerRight + distance;
        bool drewCharacter = false;

        if (left > 0) {
            const char symbol = (left % 2 == 0) ? '/' : '\\';
            draw_cell(boardX + left, bottomY, symbol);
            drewCharacter = true;
        }

        if (right < width - 1) {
            const char symbol = (right % 2 == 0) ? '/' : '\\';
            draw_cell(boardX + right, bottomY, symbol);
            drewCharacter = true;
        }

        if (!drewCharacter)
            break;

        present();
        std::this_thread::sleep_for(floorDelay);
    }

    // 3. Fill the dots left-to-right, row-by-row.
    //
    // Only odd interior columns contain dots. Even columns are spaces
    // because each logical Tetris block occupies two terminal columns.
    for (int y = 0; y < height - 2; ++y) {
        for (int x = 3; x < width - 2; x += 2) {
            draw_cell(boardX + x, boardY + y, '.');
        }
        present();
        std::this_thread::sleep_for(fillDelay);
        
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(250)
    );
}

void Game::animate_line_wipes(bool game_over, bool regame)
{
    int screen_height;
    int screen_width;

    getmaxyx(stdscr, screen_height, screen_width);

    const int board_x =
        std::max(0, (screen_width - board.get_width()) / 2);

    const int board_y =
        std::max(0, (screen_height - board.get_height()) / 2);

    const int width = board.get_width();
    const int height = board.get_height();

    const auto& built_points = board.get_built_points();

    constexpr auto line_delay = std::chrono::milliseconds(25);

    const auto present = [](){refresh();};

    if(game_over){

        for(int y = height - 3; y >= 0; --y)
        {
            for(int x = 3; x < width - 2; x += 2)
                draw_cell(board_x + x, board_y + y, ' ');
        
            refresh();
            std::this_thread::sleep_for(line_delay);
        }
    }

    if(regame){
        for (int y = 0; y < height - 2; ++y) {
            for (int x = 3; x < width - 2; x += 2) 
                draw_cell(board_x + x, board_y + y, '.');
        
            present();
            std::this_thread::sleep_for(line_delay);
        }
    }

}

void Game::animate_board_erase()
{
    int screen_height;
    int screen_width;

    getmaxyx(stdscr, screen_height, screen_width);

    const int width = board.get_width();
    const int height = board.get_height();

    const int board_x =
        std::max(0, (screen_width - board.get_width()) / 2);

    const int board_y =
        std::max(0, (screen_height - board.get_height()) / 2);

    const std::vector<Cell> built_points = board.get_built_points();
    const std::vector<std::wstring> cell_demo{L"▣▣",L"※※",L"❉❉",L"⁙⁙",L" ."};

    std::vector<std::vector<Cell>> cells_by_row(height - 2);

    if(built_points.empty())
        return;

    constexpr auto white_delay = std::chrono::milliseconds(250);
    constexpr auto collapse_delay = std::chrono::milliseconds(50);

    draw(false);


for (const Cell& tile : built_points) {
    if (tile.get_y() < 0 || tile.get_y() >= height - 2)
        continue;

    cells_by_row[tile.get_y()].push_back(tile);
}

for (std::size_t first = 0; first < cells_by_row.size(); first += 2)
{
    const std::size_t second = first + 1;

    if (cells_by_row[first].empty() &&
        (second >= cells_by_row.size() ||
         cells_by_row[second].empty()))
    {
        continue;
    }

    for (const std::wstring& frame : cell_demo)
    {
        // Animate the first row.
        for (const Cell& cell : cells_by_row[first])
        {
            draw_cell(
                board_x + cell.get_x(),
                board_y + cell.get_y(),
                frame.c_str()
            );
        }

        // Animate the second row when it exists.
        if (second < cells_by_row.size())
        {
            for (const Cell& cell : cells_by_row[second])
            {
                draw_cell(
                    board_x + cell.get_x(),
                    board_y + cell.get_y(),
                    frame.c_str()
                );
            }
        }

        // Present both rows simultaneously.
        refresh();
        std::this_thread::sleep_for(collapse_delay);
    }
}
}
