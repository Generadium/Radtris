#include "Game.h"
#include <algorithm>
#include <string>
#include <vector>
#include <clocale>

int main(int argc, char ** argv) 
{
    //game menu logic
    std::setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    // set game to black and green 
    if(has_colors())
    {
        start_color();

        init_pair(1, COLOR_GREEN, COLOR_BLACK);

        bkgd(COLOR_PAIR(1));
        attrset(COLOR_PAIR(1));

        erase();
        refresh();
    }
    
    const std::string title = "RADTRIS";
    const std::vector<std::string> menu = {
        "Start","Quit"
    };

    int menu_width = (static_cast<int>(menu.size()) - 1);
    for (const std::string& option : menu)
    {
        menu_width += static_cast<int>(option.size());
    }

    int selected = 0;
    bool run = true;

    while(run)
    {
        erase();

        int screen_height;
        int screen_width;
        getmaxyx(stdscr, screen_height, screen_width);

        const int menu_height = static_cast<int>(menu.size()) + 2;
        const int start_y = std::max(0, (screen_height - menu_height) / 2);
        int menu_x = std::max(0, (screen_width - menu_width) / 2);

        int title_x =
            std::max(0, (screen_width - static_cast<int>(title.size())) / 2);

        attron(A_BOLD);
        mvprintw(start_y - 1, title_x, "[]");
        mvprintw(start_y, title_x, "%s", title.c_str());
        mvprintw(start_y + 1, title_x + 5, "[]");
        attron(A_BOLD);

        for (std::size_t i = 0; i < menu.size(); ++i)
        {
            int option_x = std::max(
                0,
                (screen_width - static_cast<int>(menu[i].size())) / 2
            );

            if (static_cast<int>(i) == selected)
                attron(COLOR_PAIR(1) | A_REVERSE);

            mvprintw(
                start_y + 4,
                menu_x,
                "%s",
                menu[i].c_str()
            );

            if (static_cast<int>(i) == selected)
                attroff(COLOR_PAIR(1) | A_REVERSE);

            menu_x += static_cast<int>(menu[i].size()) + menu_width;
        }

        refresh();

        int input = getch();

        switch (input)
        {
            case KEY_LEFT:
                if(selected > 0)
                    --selected;
                break;

            case KEY_RIGHT:
                if (selected < static_cast<int>(menu.size()) - 1)
                    ++selected;
                break;

            case '\n':
            case KEY_ENTER:
                if (selected == 0) {
                    Game game;
                    run = false;
                }
                else if (selected == 1)
                    run = false;
                break;

            case 'q':
            case 'Q':
                run = false;
                break;
        }
    }

    endwin();
    return 0;
}
