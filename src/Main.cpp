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

bool initializeColors()
{
    if (!has_colors() || start_color() == ERR)
        return false;


    if (use_default_colors() == ERR)
        return false;

    return init_pair(1, COLOR_GREEN, COLOR_BLACK) != ERR &&
           init_pair(2, COLOR_BLACK, COLOR_WHITE) != ERR;
}

void applyColorSettings(ColorSettings settings)
{
    chtype color = COLOR_PAIR(0);

    switch (settings)
    {
        case ColorSettings::Default:
            break;

        case ColorSettings::BlackAndGreen:
            color = COLOR_PAIR(1);
            break;

        case ColorSettings::WhiteAndBlack:
            color = COLOR_PAIR(2);
            break;
    }

    attrset(A_NORMAL);
    bkgd(' ' | color);
    attrset(color);
    erase();
    refresh();
}

void showSettings(ColorSettings& currentColorSetting, bool colorsAvailable)
{
    const std::vector<std::string> options = {
        "Terminal default",
        "Electronika 60 (Black and Green)",
        "White and Black",
        "Back"
    };
    const int backIndex = static_cast<int>(options.size()) - 1;
    int selected = static_cast<int>(currentColorSetting);

    while (true)
    {
        erase();

        int screenHeight, screenWidth;
        getmaxyx(stdscr, screenHeight, screenWidth);
        const int startY = std::max(0, (screenHeight - 10) / 2);
        const int startX = std::max(0, (screenWidth - 41) / 2);

        attron(A_BOLD);
        mvprintw(startY, startX, "Graphics mode");
        attroff(A_BOLD);

        for (int i = 0; i < static_cast<int>(options.size()); ++i)
        {
            if (i == selected)
                attron(A_REVERSE);

            const bool active = i == static_cast<int>(currentColorSetting);
            const std::string label = options[i] + (active ? " [active]" : "");
            mvaddnstr(startY + 2 + i, startX, label.c_str(), screenWidth - startX);

            if (i == selected)
                attroff(A_REVERSE);
        }

        if (!colorsAvailable)
            mvaddnstr(startY + 7, startX, "Color modes unavailable in this terminal.",
                      screenWidth - startX);

        mvaddnstr(startY + 9, startX, "Up/Down: choose  Enter: apply  Esc/Q: back",
                  screenWidth - startX);
        refresh();

        switch (getch())
        {
            case KEY_UP:
                selected = (selected + backIndex) % static_cast<int>(options.size());
                break;

            case KEY_DOWN:
                selected = (selected + 1) % static_cast<int>(options.size());
                break;

            case '\r':
            case '\n':
            case KEY_ENTER:
                if (selected == backIndex)
                    return;

                if (selected != 0 && !colorsAvailable)
                    break;

                currentColorSetting = static_cast<ColorSettings>(selected);
                applyColorSettings(currentColorSetting);
                break;

            case 27: // Escape
            case 'q':
            case 'Q':
                return;
        }
    }
}

int main(int argc, char ** argv) 
{
    //game menu logic
    std::setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    const bool colorsAvailable = initializeColors();
    ColorSettings currentColorSetting = ColorSettings::Default;
    applyColorSettings(currentColorSetting);
    
    const std::string title = "RADTRIS";
    const std::vector<std::string> menu = {
        "Start","Settings","Quit"
    };
    const int menu_gap = 4;
    int menu_width = (static_cast<int>(menu.size()) - 1) * menu_gap;
    for (const std::string& option : menu)
    {
        menu_width += static_cast<int>(option.size());
    }

    int menu_selected = 0;
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
        attroff(A_BOLD);

        for (std::size_t i = 0; i < menu.size(); ++i)
        {
            if (static_cast<int>(i) == menu_selected)
                attron(A_REVERSE);

            mvprintw(
                start_y + 4,
                menu_x,
                "%s",
                menu[i].c_str()
            );

            if (static_cast<int>(i) == menu_selected)
                attroff(A_REVERSE);

            menu_x += static_cast<int>(menu[i].size()) + menu_gap;
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

            case '\r':
            case '\n':
            case KEY_ENTER:
                if (menu_selected == 0) //Start
                {
                    Game game;
                    run = false;
                }
                else if (menu_selected == 1) //Settings
                {
                    showSettings(currentColorSetting, colorsAvailable);
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
