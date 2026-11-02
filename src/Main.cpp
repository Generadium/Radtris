#include "Game.h"
#include <algorithm>
#include <string>
#include <vector>
#include <clocale>

enum class ColorSettings {
        Default,
        BlackAndGreen,
        WhiteAndBlack
    };

void applyColorSettings(ColorSettings settings)
    {
        if (has_colors())
        {
            start_color();

            switch (settings)
            {
                case ColorSettings::Default:
                    init_pair(1, COLOR_WHITE, COLOR_BLACK);
                    break;

                case ColorSettings::BlackAndGreen:
                    init_pair(1, COLOR_GREEN, COLOR_BLACK);
                    break;

                case ColorSettings::WhiteAndBlack:
                    init_pair(1, COLOR_BLACK, COLOR_WHITE);
                    break;
            }

            bkgd(COLOR_PAIR(1));
            attrset(COLOR_PAIR(1));

            erase();
            refresh();
        }
    };

int main(int argc, char ** argv) 
{
    //game menu logic
    std::setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    
    const std::string title = "RADTRIS";
    const std::vector<std::string> menu = {
        "Start","Settings","Quit"
    };
    const std::vector<std::string> settings = {
        "Default","Electronika 60 (Black and Green)","White and Black"
    };

    int menu_width = (static_cast<int>(menu.size()) - 1);
    for (const std::string& option : menu)
    {
        menu_width += static_cast<int>(option.size());
    }

    int menu_selected = 0;
    int settings_selected = 0;
    bool run = true;

    ColorSettings currentColorSetting = ColorSettings::Default;

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

            if (static_cast<int>(i) == menu_selected)
                attron(COLOR_PAIR(1) | A_REVERSE);

            mvprintw(
                start_y + 4,
                menu_x,
                "%s",
                menu[i].c_str()
            );

            if (static_cast<int>(i) == menu_selected)
                attroff(COLOR_PAIR(1) | A_REVERSE);

            menu_x += static_cast<int>(menu[i].size()) + menu_width;
        }

        refresh();

        int input = getch();

        switch (input)
        {
            case KEY_LEFT:
                if(menu_selected > 0)
                    --menu_selected;
                break;

            case KEY_RIGHT:
                if (menu_selected < static_cast<int>(menu.size()) - 1)
                    ++menu_selected;
                break;

            case '\n':
            case KEY_ENTER:
                if (menu_selected == 0) //Start
                {
                    Game game;
                    run = false;
                }
                else if (menu_selected == 1) //Settings
                {
                    erase();

                    for(int i = 0; i < static_cast<int>(settings.size()); ++i)
                    {
                        int option_x = std::max(
                            0,
                            (screen_width - static_cast<int>(settings[i].size())) / 2
                        );

                        if (i == settings_selected)
                            attron(COLOR_PAIR(1) | A_REVERSE);

                        mvprintw(
                            start_y + 4 + i,
                            menu_x,
                            "%s",
                            settings[i].c_str()
                        );

                        if (i == settings_selected)
                            attroff(COLOR_PAIR(1) | A_REVERSE);
                    }

                    switch (input)
                    {
                        case KEY_LEFT:
                        if(settings_selected > 0)
                            --settings_selected;
                        break;

                    case KEY_RIGHT:
                        if (settings_selected < static_cast<int>(settings.size()) - 1)
                            ++settings_selected;
                        break;

                    case '\n':

                    case KEY_ENTER:
                        currentColorSetting = static_cast<ColorSettings>(settings_selected);
                        applyColorSettings(currentColorSetting);
                        break;
                    }

                    refresh();

                }
                else if (menu_selected == 2) //Quit
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
