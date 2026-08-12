#include "Game.h"

Game::Game() // Game constructor
    : run {true}
{
    std::setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    bool again = false;
    do // do while loop, run the game while regame is still true
    {
        board = Board(24, 22); // initializes the board dimensions using the board class constructor
        
        score = 0;
        lines_built = 0;
        piece_count = 0;
        level = 0;
        paused = false;
        piece_active = false;
        lock_active = false;
        slam_request = false;

        erase();
        refresh();

        running();
        again = regame();
    
    } while (again);

    refresh();
    endwin();
    std::cout << "\033[2J\033[H" << std::flush;

}

//function wrappers for drawing characters on the screen 
void Game::draw_cell(int x, int y, char ch){
    mvaddch(y, x, ch);
}

void Game::draw_cell(int x, int y, const char *text){
    mvprintw(y, x, "%s", text);
}

void Game::running() //main game loop
{
    next = generate_piece_type(); // next piece is preemptively generated

    while (!game_over()) //while it is not game over
    {
        piece = Piece(Cell(board.get_width() / 2, board.get_height() - 24, 'O'), next); //piece generated

        next = generate_piece_type();
        piece_active = true;
        lock_active = false;
        last_fall_time = Clock::now();

        while (piece_active)
        {
            Input input = readInput();

            if (input == Input::pause)
            {
                paused = !paused;

                if(!paused)
                {
                    const auto resumed_at = Clock::now();
                    last_fall_time = resumed_at;

                    if(lock_active)
                        grounded_since = resumed_at;
                }
            }

            if(paused)
            {
                draw();
                getmaxyx(stdscr, screen_height, screen_width);
                int board_x = std::max(0, (screen_width - board.get_width()) / 2);

                draw_cell((board_x - 27),2,"PAUSED");
                draw_cell((board_x - 27),3,"Press p to resume");
                refresh();

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const auto now = Clock::now();
            Result result = readResult(input);

            if(!piece_active)
                break;

            if(now - last_fall_time >= grav_interval)
            {
                checked_fall();
                last_fall_time = now;
            }

            update_lock_delay(result, Clock::now());
            refresh_board();
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        refresh_board();

        const auto complete_rows = board.get_completed_rows();
        if(!complete_rows.empty())
        {
            //modify here?
            board.remove_row(complete_rows);
            lines_built += static_cast<int>(complete_rows.size());
            score = score_calc(complete_rows, level);
            refresh_board();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            board.points_fall(complete_rows);
            refresh_board();
        }
    }
}

int Game::score_calc(std::vector<int> completed_rows, int level)
{
    if(completed_rows.size() == 1)
        score += 40*(level + 1);
    if(completed_rows.size() == 2)
        score += 100*(level + 1);
    if(completed_rows.size() == 3)
        score += 300*(level + 1);
    if(completed_rows.size() == 4)
        score += 1200*(level + 1);
    if(blocksland())
        score += 5;

    return score;
}

bool Game::game_over() //declares the game over
{
    for (const auto& pnt : board.get_built_points()) // points to all built tiles in the game
        if (pnt.get_y() <= 0)
        {
            return true;
        }
    return false;
}

bool Game::regame() //game over screen, asks the player to play again
{
    board_x = std::max(0, (screen_width - board.get_width()) / 2);
    board_y = std::max(0, (screen_height - board.get_height()) / 2);

    nodelay(stdscr, FALSE);
    flushinp();

    getmaxyx(stdscr, screen_height, screen_width);

    draw_cell(board_x - 27, 2, "GAME OVER");
    refresh();

    leaderboard();
    draw_cell(board_x - 27, 5, "Want to play again? (y/n)");
    draw_cell(board_x - 27, 6, "Press C to clear");
    draw_cell(board_x - 27, 7, "the leaderboard");

    while (true)
    {
        const int c = getch();

        if(c == 'c' || c == 'C')
        {
            user_entry.clear_entries();
            user_entry.erase_file();

            for(int y = board.get_height(); y > 0; --y)
                mvhline(y, board_x+2, ' ', board.get_width() - 4);

            draw_cell(board_x + (board.get_width() / 2) - 4, board.get_height()/2, "Cleared!");

            refresh();
            continue;
        }
        if (c == 'y' || c == 'Y') // if 'y' is entered, game restarts
        {
            nodelay(stdscr, TRUE);

            return true;
        }
        if (c == 'n' || c == 'N') // if 'n' is entered, program terminates
            return false; 
    }

}

bool Game::blocksland() // sends the bodies of pieced that land to built points when they land 
{
    for (const auto& next_piece_pnt : piece.fallen_body()) //const pointer that points to piece.next_fall_down_body()
    {
        //Sets the piece to hit the ground
        if (next_piece_pnt.get_y() == 20)  //if piece hits y = 1
            return true;
        
        //Sets piece to collide with built tiles
        for (auto& built_pnt : board.get_built_points()) // if piece lands on top of a built piece
            if (next_piece_pnt == built_pnt) 
                return true;
              
    }

    return false;
}

bool Game::checked_move(enum move_direction dir)
{
    for (const auto& next_piece_pnt : piece.moved_body(dir))
    { 
        if (next_piece_pnt.get_x() <= 1 || next_piece_pnt.get_x() >= (board.get_width() - 3)) //prohibits pieces from going through the borders
            return false;

        for (const auto& built_pnt : board.get_built_points()) // does not allow built points to move
            if (built_pnt == next_piece_pnt)
                return false;
    }

    piece.move(dir); //otherwise, piece allowed to move
    return true;
}

bool Game::checked_rotate() 
{
    for (const auto& next_piece_pnt : piece.rotated_body()) // pointer to the rotated body of the current piece
    {
        if (next_piece_pnt.get_x() <= 1 || next_piece_pnt.get_x() >= (board.get_width() - 2)) //pieces on the border cannot rotate
            return false;

        for (const auto& built_pnt : board.get_built_points()) // does not allow built points to rotate
            if (built_pnt == next_piece_pnt)
                return false;
    }
    piece.rotate(); //otherwise, piece allowed to rotate
    return true;
}

bool Game::can_fall()
{
    for(const auto& next_piece_pnt : piece.fallen_body())
    {
        if(next_piece_pnt.get_y() >= board.get_height() - 2) //if next position contains built points
            return false;

        for(const auto& built_point : board.get_built_points())
            if(next_piece_pnt == built_point)
                return false;
    }

    return true;
}

bool Game::checked_fall()
{
    if(!can_fall())
        return false;
    piece.fall_down();
    return true;
}

void Game::draw(bool present) //creates the board
{
    erase();

    getmaxyx(stdscr, screen_height, screen_width);

    int board_x = std::max(0, (screen_width - board.get_width()) / 2);
    int board_y = std::max(0, (screen_height - board.get_height()) / 2);

    for (int i = (board.get_height() - 1); i >= 0; --i) // double nested for loop
    { 
        for (int j = 0; j < board.get_width(); ++j) {
            auto t = find(final_points.begin(), final_points.end(), Cell(j, i)); // uses find function to find all final points within range of for loop
            if(t == final_points.end()) continue;
            if(t->get_type() == 'O'){
                draw_cell(board_x + j, board_y + i, "[]");
                ++j;
            } else {
                draw_cell(board_x + j, board_y + i, t->get_type());
            }
        }    
    }
    Piece preview(Cell(board_x + board.get_width() + 6, board_y + 8, 'O'), next);

    draw_cell(board_x + (board.get_width() / 2) - 3, board_y - 1,"RADTRIS");
    draw_cell((board_x + board.get_width() + 2), board_y + 1, ("Score: " + to_string(score)).c_str());
    draw_cell((board_x + board.get_width() + 2), board_y + 2, ("Lines built: " + to_string(lines_built)).c_str());
    draw_cell((board_x + board.get_width() + 2), board_y + 6, "Next piece: ");
    draw_cell((board_x - 27),9,"Controls: ");

    int control_y = 12;
    for(const Controls& controls : controls){
        std::string line = 
        std::string(controls.key) + " : " + controls.action;
        draw_cell((board_x - 27), control_y++, line.c_str());
    }

    for (const auto& cell : preview.get_body())
        draw_cell(cell.get_x(), cell.get_y(), "[]");
    
    if(present)
        refresh();
}

void Game::refresh_board()
{
    board.refresh();
    final_points = board.get_all_points(); // gathers all tiles
    if(piece_active)
    {
    for (auto &final_pnt : final_points) // pointer to all cells at the end of a game
        for (auto piece_pnt : piece.get_body()) // sets all built cells to final cells
            if (final_pnt == piece_pnt)
                final_pnt = piece_pnt;
    }
    draw();
}

void Game::lock()
{
    board.insert_to_built_points(piece.get_body());//sends current piece to built points (tiles that have been built)
    piece_count++; // adds to piece count
    piece_active = false;
    lock_active = false;
}

void Game::update_lock_delay(Game::Result result, Clock::time_point now)
{
    if(can_fall())
    {
        lock_active = false;
        return;
    }

    if(!lock_active)
    {
        lock_active = true;
        grounded_since = now;
        return;
    }

    if (result == Result::move || result == Result::rotate)
        grounded_since = now;

    if(now - grounded_since >= lock_delay)
        lock();
}

piece_type Game::generate_piece_type()
{
    if(bag.empty())
        refill();

    piece_type next = bag.back();
    bag.pop_back();

    return next;
}

void Game::refill()
{
    bag = {
        t_piece,
        i_piece,
        o_piece,
        l_piece,
        j_piece,
        s_piece,
        z_piece
    };

    std::shuffle(
        bag.begin(),
        bag.end(),
        randomEngine
    );
}

Game::Input Game::readInput(){
 
    int ch = getch();

    switch (ch){
        case KEY_UP: return Input::up;
        case KEY_DOWN: return Input::down;
        case KEY_RIGHT: return Input::left;
        case KEY_LEFT: return Input::right;
        case ' ': return Input::space;
        case 'p':
        case 'P': return Input::pause;
        case KEY_RESIZE: clearok(stdscr, TRUE); return Input::none;
        default: return Input::none;
    }

}

Game::Result Game::readResult(Input key)
{
    switch(key)
    {
        case Input::left:
            return checked_move(move_direction::left)
                ? Result::move : Result::none;
        
        case Input::right:
            return checked_move(move_direction::right)
                ? Result::move : Result::none;

        case Input::up:
            return checked_rotate()
                ? Result::rotate : Result::none;

        case Input::down:
            // if(piece_active)
            //     score++;
            return checked_fall()
                ? Result::soft_drop : Result::none;

        case Input::space:
            while (checked_fall()){score++;}
            lock();
            return Result::slam;

        default:
            return Result::none;
    }
}

void Game::leaderboard() //leaderboard game logic
{


    nodelay(stdscr, FALSE);
    echo(); //displays user's input name
    curs_set(1); //creates typehead

    draw_cell((board_x - 27), 3, "Enter your name: ");
    move(3, (board_x - 10));//position cursor
    refresh();

    auto scores = user_entry.read_scores();
    //vector operations
    user_entry.new_entry.name = user_entry.enter_name(); //name entered
    user_entry.new_entry.score_num = score; //score entered

    //push new score to score vector and resort
    scores.push_back(user_entry.new_entry); // user score pushed to entries vector
    std::sort(scores.begin(),scores.end(),
    [](const auto& a, const auto& b){
        return a.score_num > b.score_num;
    });

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    board.clear_built_points();
    board.refresh();
    final_points = board.get_all_points();

    for(auto& point : final_points)
    {
        if(point.get_type() == '.')
            point.set_type(' ');
    }
    draw();

    //file-writing / display logic goes here
    const int left_edge = board_x + 2;
    const int right_edge = board_x + board.get_width() - 3;
    int y_pos = 3;

    draw_cell(board_x + (board.get_width() / 2) - 4, 2, "TOP SCORES");
    //display scores on screen
    for(auto& entry : scores)
    {
        int score_x = right_edge - static_cast<int>(std::to_string(entry.score_num).length()) + 1;
        draw_cell(left_edge, y_pos, entry.name.c_str()); 
        draw_cell(score_x, y_pos, std::to_string(entry.score_num).c_str()); //change position
        ++y_pos;
    }

    refresh();

    //save to file and clear both vectors
    user_entry.save(scores);
    scores.clear();
    user_entry.clear_entries();
}

void Leaderboard::add_score(const Score& score)
{
    entries.push_back(score); //add entries to the entry vector
}

void Leaderboard::save(std::vector<Score> scores) const
{
    std::ofstream file("score/scores.csv", std::ios::trunc); //open scores.csv

    if(!file)
        return;

    for (const Score& score : scores) //saves all scores in entries to scores.csv
        file << score.name << ',' << score.score_num << '\n';

    file.close(); //close file
}

std::vector<Leaderboard::Score> Leaderboard::read_scores()
{
    Score entry;

    std::ifstream file("score/scores.csv"); //read from scores.csv
    if(!file.is_open())
        return {};

    std::string line;
    while(std::getline(file, line)) //while there are lines
    {
        std::string name;
        std::string num_string;
        int num = 0;

        std::stringstream ss(line);
        std::vector<std::string> names;
        std::vector<std::string> score_nums;

        if(std::getline(ss, name, ','))
        {
            try
            {entry.name = name;}
            catch(const std::exception& e){}
            
            if(std::getline(ss, num_string, ','))
            {
               try
               {
                    num = std::stoi(num_string);
                    entry.score_num = num;
               }
               catch(const std::exception& e)
               {}
               
            }
        }
        add_score(entry); //place scores from file into vector
    }

    file.close(); //close file

    return entries;
}

std::string Leaderboard::enter_name() //user enters name into leaderboard
{

    getnstr(user_buf.data(), MAX_LEN); // user inputs name into buffer

    std::string name{user_buf.data()}; // buffer converted into string
    
    return name;
}

void Leaderboard::clear_entries()
{
    entries.clear();
}

void Leaderboard::erase_file()
{
    std::ofstream file("score/scores.csv", std::ios::trunc);
    if(!file)
        return;

    file.close();
}