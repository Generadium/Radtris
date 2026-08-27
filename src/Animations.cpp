#include "Game.h"
#include "Board.h"

void Game::draw_clear_frame(
    const std::vector<int>& rows,
    int dissolvedDistance,
    bool flashOn)
{

    int screenHeight;
    int screenWidth;

    getmaxyx(stdscr, screenHeight, screenWidth);

    const int board_x =
        std::max(0, (screenWidth - board.get_width()) / 2);

    const int board_y =
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

            const int cellX = 2 + block * 2;

            if (dissolved || !flashOn) {
                draw_cell(
                    board_x + cellX,
                    board_y + row,
                    " ."
                );
            } else {
                white_block(
                    board_x + cellX,
                    board_y + row
                );
            }
        }
    }

    wnoutrefresh(stdscr);
    doupdate();
}

void Game::white_block(int x, int y)
{
    attron(A_REVERSE | A_BOLD);
    mvaddstr(y, x, "  ");
    attroff(A_REVERSE | A_BOLD);

}

void Game::animate_line(const std::vector<int>& rows)
{
    draw(false);
    wnoutrefresh(stdscr);
    doupdate();

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

    draw_clear_frame(rows, dissolveSteps, false);
}

void Game::animate_board_init()
{
    int screenHeight;
    int screenWidth;

    getmaxyx(stdscr, screenHeight, screenWidth);
    erase();


    const int board_x = std::max(0, (screenWidth - board.get_width()) / 2);

    const int board_y = std::max(0, (screenHeight - board.get_height()) / 2);

    const int width = board.get_width();
    const int height = board.get_height();

    constexpr auto wallDelay = std::chrono::milliseconds(25);
    constexpr auto floorDelay = std::chrono::milliseconds(35);
    constexpr auto fillDelay = std::chrono::milliseconds(25);

    const auto present = []() {
    wnoutrefresh(stdscr);
    doupdate();

    };

    for (int y = 0; y < height - 1; ++y) {
        draw_cell(board_x,             board_y + y, '<');
        draw_cell(board_x + 1,         board_y + y, '!');
        draw_cell(board_x + width - 2, board_y + y, '!');
        draw_cell(board_x + width - 1, board_y + y, '>');

        present();
        std::this_thread::sleep_for(wallDelay);
    }

    // Draw the row directly above the patterned bottom border.
    draw_cell(board_x, board_y + height - 2, '<');

    for (int x = 2; x < width - 2; ++x)
        draw_cell(board_x + x, board_y + height - 2, '=');

    draw_cell(
        board_x + width - 1,
        board_y + height - 2,
        '>'
    );

    present();

    for(int x = 1; x < width - 1; ++x)
    {
        const char bottom = (x % 2 == 0) ? '/' : '\\';
        draw_cell(board_x + x, board_y + height - 1, bottom);
    }

    
    for (int y = 0; y < height - 2; ++y) {
        for (int x = 3; x < width - 2; x += 2) {
            draw_cell(board_x + x, board_y + y, '.');
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

// Animate one row per wave. With a one-third-duration stagger, up to three
// separate row waves overlap at once.
constexpr std::size_t rows_per_wave = 1;

std::vector<std::size_t> waves;
for (std::size_t first = 0;
     first < cells_by_row.size();
     first += rows_per_wave) {
    bool contains_cells = false;

    for (std::size_t offset = 0; offset < rows_per_wave; ++offset) {
        const std::size_t row = first + offset;
        if (row < cells_by_row.size() && !cells_by_row[row].empty()) {
            contains_cells = true;
            break;
        }
    }

    if (contains_cells)
        waves.push_back(first);
}

if (waves.empty())
    return;

const auto wave_duration = collapse_delay * cell_demo.size();
const auto wave_stagger = wave_duration / 3;
const auto timeline_step = std::chrono::milliseconds(1);
const auto total_duration =
    wave_stagger * (waves.size() - 1) + wave_duration;

for (auto elapsed = std::chrono::milliseconds::zero();
     elapsed < total_duration;
     elapsed += timeline_step) {
    for (std::size_t wave = 0; wave < waves.size(); ++wave) {
        const auto start_time = wave_stagger * wave;
        if (elapsed < start_time || elapsed >= start_time + wave_duration)
            continue;

        const auto frame_index = static_cast<std::size_t>(
            (elapsed - start_time) / collapse_delay
        );
        const std::size_t first = waves[wave];

        for (std::size_t offset = 0; offset < rows_per_wave; ++offset) {
            const std::size_t row = first + offset;
            if (row >= cells_by_row.size())
                break;

            for (const Cell& cell : cells_by_row[row]) {
                draw_cell(board_x + cell.get_x(), board_y + cell.get_y(),
                          cell_demo[frame_index].c_str());
            }
        }
    }

    wnoutrefresh(stdscr);
    doupdate();
    std::this_thread::sleep_for(timeline_step);
}
}
