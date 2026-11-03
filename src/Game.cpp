#include "Game.h"

Game::Game() // Game constructor
    : run {true}
{

    // main() owns the terminal session and its selected color scheme.
    nodelay(stdscr, TRUE);

    animate_board_init();

    bool again = false;
    do // do while loop, run the game while regame is still true
    {
        board = Board(24, 22); // initializes the board dimensions using the board class constructor
        
        //set all initial values to zero / false
        score = 0;
        progression.reset();
        piece_count = 0;
        paused = false;
        piece_active = false;
        lock_active = false;
        slam_request = false;

        erase();
        refresh();

        //main game loop
        running();
        //if play again
        again = run && regame();
        if(again)
        {
            erase();
            board.refresh();
            draw(false);
            refresh();

            animate_line_wipes(false, again);
        }
    
    } while (again);

    refresh();

}

//function wrappers for drawing characters on the screen 
void Game::draw_cell(int x, int y, char ch){
    mvaddch(y, x, ch);
}

void Game::draw_cell(int x, int y, const char *text){
    mvprintw(y, x, "%s", text);
}

void Game::draw_cell(int x, int y, const wchar_t* text)
{
    mvaddwstr(y, x, text);
}

void Game::running() //main game loop
{
    next = generate_piece_type(); // next piece is preemptively generated

    while (!game_over()) //while it is not game over
    {
        //initialize piece
        piece = Piece(Cell(board.get_width() / 2, board.get_height() - 24, 'O'), next); //piece generated

        //next piece preview generated
        next = generate_piece_type();
        piece_active = true;
        lock_active = false;
        last_fall_time = Clock::now();
        const auto grav_interval = progression.gravity_interval();

        //while a piece is falling on the board
        while (piece_active)
        {
            //read input from user
            Input input = readInput();

            //if user pauses the game
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
                if (input == Input::quit)
                {
                    run = false;
                    return;
                }

                draw();
                getmaxyx(stdscr, screen_height, screen_width);
                int board_x = std::max(0, (screen_width - board.get_width()) / 2);

                draw_cell((board_x - 27),2,"PAUSED");
                draw_cell((board_x - 27),3,"Press p to resume");
                draw_cell((board_x - 27),4,"Press q to quit");
                refresh();

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const auto now = Clock::now();
            Result result = readResult(input);

            //if piece is no longer active, break from loop
            if(!piece_active)
                break;

            // Preserve elapsed time so polling does not flatten the faster levels.
            while(now - last_fall_time >= grav_interval)
            {
                last_fall_time += grav_interval;
                if (!checked_fall())
                {
                    last_fall_time = now;
                    break;
                }
            }

            update_lock_delay(result, Clock::now());
            refresh_board();
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        refresh_board();

        //remove row and push blocks down afterward, increase score
        const auto complete_rows = board.get_completed_rows();
        if(!complete_rows.empty())
        {
            animate_line(complete_rows);
            board.remove_row(complete_rows);
            progression.add_lines(static_cast<int>(complete_rows.size()));
            score = score_calc(complete_rows, progression.level());
            refresh_board();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

bool Game::game_over() 
{
    for (const auto& pnt : board.get_built_points()) //if blocks reach top of the board, game is over
        if (pnt.get_y() <= 0)
        {
            return true;
        }
    return false;
}

bool Game::regame() //if player wants to play again
{
    board_x = std::max(0, (screen_width - board.get_width()) / 2);
    board_y = std::max(0, (screen_height - board.get_height()) / 2);

    nodelay(stdscr, FALSE);
    flushinp();

    getmaxyx(stdscr, screen_height, screen_width);

    draw_cell(board_x - 27, 2, "GAME OVER");
    refresh();

    leaderboard();
    draw_cell(board_x - 27, 4, "Want to play again? (y/n)");
    draw_cell(board_x - 27, 6, "Press C to clear");
    draw_cell(board_x - 27, 7, "the leaderboard");

    while (true)
    {
        const int c = getch();

        //if player wants to clear top scores
        if(c == 'c' || c == 'C')
        {
            user_entry.clear_entries();
            user_entry.erase_file();

            for (int row = 2; row < board.get_height() - 2; ++row)
                mvhline(board_y + row, board_x + 2, ' ', board.get_width() - 4);

            draw_cell(board_x + (board.get_width() / 2) - 4, board_y + (board.get_height() / 2), "Cleared!");
            
            refresh();
            continue;
        }
        if (c == 'y' || c == 'Y') //play again
        {
            nodelay(stdscr, TRUE);

            return true;
        }
        if (c == 'n' || c == 'N') //quit
            return false; 
    }

}

bool Game::blocksland() //if piece lands on y = 1 or a built tile, convert it to a built tile
{
    for (const auto& next_piece_pnt : piece.fallen_body()) 
    {
        
        if (next_piece_pnt.get_y() == 20)  
            return true;
        
        for (auto& built_pnt : board.get_built_points()) 
            if (next_piece_pnt == built_pnt) 
                return true;
              
    }

    return false;
}

bool Game::checked_move(enum move_direction dir) // is the piece allowed to move?
{
    for (const auto& next_piece_pnt : piece.moved_body(dir))
    { 
        if (next_piece_pnt.get_x() <= 1 || next_piece_pnt.get_x() >= (board.get_width() - 3)) //if piece is next to a border, no
            return false;

        for (const auto& built_pnt : board.get_built_points()) // if piece is next to a built tile, no
            if (built_pnt == next_piece_pnt)
                return false;
    }
    //otherwise, yes
    piece.move(dir); 
    return true;
}

bool Game::checked_rotate() // is the piece allowed to rotate?
{
    for (const auto& next_piece_pnt : piece.rotated_body())
    {
        if (next_piece_pnt.get_x() <= 1 || next_piece_pnt.get_x() >= (board.get_width() - 2)) //if piece is next to a border, no
            return false;

        for (const auto& built_pnt : board.get_built_points()) // if piece is next to built points, no
            if (built_pnt == next_piece_pnt)
                return false;
    }
    //otherwise, yes
    piece.rotate(); 
    return true;
}

bool Game::can_fall() // is the piece falling?
{
    for(const auto& next_piece_pnt : piece.fallen_body())
    {
        if(next_piece_pnt.get_y() >= board.get_height() - 2) //if piece is at y = 1, no
            return false;

        for(const auto& built_point : board.get_built_points()) // if piece is above built points, no
            if(next_piece_pnt == built_point)
                return false;
    }
    //otherwise, yes
    return true;
}

bool Game::checked_fall()
{
    if(!can_fall())
        return false;
    piece.fall_down();
    return true;
}

void Game::draw(bool present) //creates the board and messages
{
    erase();

    getmaxyx(stdscr, screen_height, screen_width);

    int board_x = std::max(0, (screen_width - board.get_width()) / 2);
    int board_y = std::max(0, (screen_height - board.get_height()) / 2);

    for (int i = (board.get_height() - 1); i >= 0; --i) 
    { 
        for (int j = 0; j < board.get_width(); ++j) {
            auto t = find(final_points.begin(), final_points.end(), Cell(j, i)); //builds the body of the piece
            if(t == final_points.end()) continue;
            if(t->get_type() == 'O'){ // draws piece first with O
                draw_cell(board_x + j, board_y + i, "[]"); // then draws [] cells to create block characters
                ++j;
            } else {
                draw_cell(board_x + j, board_y + i, t->get_type()); //otherwise, draw everything according to its char type
            }
        }    
    }
    Piece preview(Cell(board_x + board.get_width() + 6, board_y + 8, 'O'), next); // draw preview piece in slot

    attron(A_BOLD);
    draw_cell(board_x + (board.get_width() / 2) - 3, board_y - 1,"RADTRIS"); // game title
    attron(A_BOLD);
    draw_cell((board_x + board.get_width() + 2), board_y + 1, ("Score: " + to_string(score)).c_str()); // display score
    draw_cell((board_x + board.get_width() + 2), board_y + 2, ("Lines built: " + to_string(progression.lines())).c_str()); // display lines built
    draw_cell((board_x + board.get_width() + 2), board_y + 4, ("Level: " + to_string(progression.level())).c_str());
    draw_cell((board_x + board.get_width() + 2), board_y + 6, "Next piece: "); // preview slot
    draw_cell((board_x - 27),9,"Controls: "); // display controls

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

void Game::refresh_board() // update char types of board characters
{
    board.refresh();
    final_points = board.get_all_points(); 
    if(piece_active)
    {
    for (auto &final_pnt : final_points) 
        for (auto piece_pnt : piece.get_body()) 
            if (final_pnt == piece_pnt)
                final_pnt = piece_pnt;
    }
    draw();
}

void Game::lock() // lock piece into place
{
    board.insert_to_built_points(piece.get_body());//converts active piece body into built tiles
    piece_count++; 
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

piece_type Game::generate_piece_type() // randomly generate the piece type
{
    if(bag.empty())
        refill();

    piece_type next = bag.back();
    bag.pop_back();

    return next;
}

void Game::refill() //refill the piece type bag
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

Game::Input Game::readInput()// read input from user
{
 
    int ch = getch();

    switch (ch){
        case KEY_UP: return Input::up;
        case KEY_DOWN: return Input::down;
        case KEY_RIGHT: return Input::left;
        case KEY_LEFT: return Input::right;
        case ' ': return Input::space;
        case 'p':
        case 'P': return Input::pause;
        case 'q':
        case 'Q': return Input::quit;
        case KEY_RESIZE: clearok(stdscr, TRUE); return Input::none;
        default: return Input::none;
    }

}

Game::Result Game::readResult(Input key) // results from user input
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
    // vector operations
    user_entry.new_entry.name = user_entry.enter_name(); // name entered
    user_entry.new_entry.score_num = score; //score entered

    // push new score to score vector and resort
    scores.push_back(user_entry.new_entry); // user score pushed to entries vector
    std::sort(scores.begin(),scores.end(),
    [](const auto& a, const auto& b){
        return a.score_num > b.score_num;
    });

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    animate_board_erase();
    board.clear_built_points();
    animate_line_wipes(true, false);

    board.refresh();
    final_points = board.get_all_points();

    for(auto& point : final_points)
    {
        if(point.get_type() == '.')
            point.set_type(' ');
    }
    draw();

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

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
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
