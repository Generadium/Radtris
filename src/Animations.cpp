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

#ifndef _WIN32
    getmaxyx(stdscr, screenHeight, screenWidth);
#else
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(
        GetStdHandle(STD_OUTPUT_HANDLE),
        &info
    );

    screenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    screenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;
#endif

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
                DrawAt(
                    boardX + cellX,
                    boardY + row,
                    " ."
                );
            } else {
                DrawWhiteBlockAt(
                    boardX + cellX,
                    boardY + row
                );
            }
        }
    }

#ifndef _WIN32
    refresh();
#else
    std::cout.flush();
#endif
}

void Game::DrawWhiteBlockAt(int x, int y)
{
#ifndef _WIN32
    attron(A_REVERSE | A_BOLD);
    mvaddstr(y, x, "  ");
    attroff(A_REVERSE | A_BOLD);
#else
    DrawAt(x, y, "██");
#endif
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

#ifndef _WIN32
    getmaxyx(stdscr, screenHeight, screenWidth);
    erase();
#else
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(
        GetStdHandle(STD_OUTPUT_HANDLE),
        &info
    );

    screenWidth = info.srWindow.Right - info.srWindow.Left + 1;
    screenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;

    ClearScreen();
#endif

    const int boardX =
        std::max(0, (screenWidth - board.get_width()) / 2);

    const int boardY =
        std::max(0, (screenHeight - board.get_height()) / 2);

    const int width = board.get_width();
    const int height = board.get_height();

    constexpr auto wallDelay = std::chrono::milliseconds(25);
    constexpr auto floorDelay = std::chrono::milliseconds(35);
    constexpr auto fillDelay = std::chrono::milliseconds(8);

    // Present everything currently drawn.
    const auto present = []() {
#ifndef _WIN32
        refresh();
#else
        std::cout.flush();
#endif
    };

    // 1. Draw the vertical walls from top to bottom.
    //
    // Board columns:
    // x == 0         contains '<'
    // x == 1         contains '!'
    // x == width - 2 contains '!'
    // x == width - 1 contains '>'
    for (int y = 0; y < height - 2; ++y) {
        DrawAt(boardX,             boardY + y, '<');
        DrawAt(boardX + 1,         boardY + y, '!');
        DrawAt(boardX + width - 2, boardY + y, '!');
        DrawAt(boardX + width - 1, boardY + y, '>');

        present();
        std::this_thread::sleep_for(wallDelay);
    }

    // Draw the row directly above the patterned bottom border.
    DrawAt(boardX, boardY + height - 2, '<');

    for (int x = 1; x < width - 1; ++x)
        DrawAt(boardX + x, boardY + height - 2, '=');

    DrawAt(
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
            DrawAt(boardX + left, bottomY, symbol);
            drewCharacter = true;
        }

        if (right < width - 1) {
            const char symbol = (right % 2 == 0) ? '/' : '\\';
            DrawAt(boardX + right, bottomY, symbol);
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
            DrawAt(boardX + x, boardY + y, '.');

            present();
            std::this_thread::sleep_for(fillDelay);
        }
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(250)
    );
}