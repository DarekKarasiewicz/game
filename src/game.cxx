#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <thread>

#include <darek_game/map.h>

using pos_type = std::pair<size_t, size_t>;
namespace {
auto write(int fd, std::string const s) -> ssize_t
{
    return ::write(fd, s.c_str(), s.size());
}
}  // namespace

auto clear() -> void
{
    write(1, "\033[2J", 4);
}

auto set_cursor(int x, int y) -> void
{
    auto set = "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1, set.c_str(), set.size());
}

auto hide_cursor() -> void
{
    auto set = std::string{"\e[?25l"};
    write(1, set.c_str(), set.size());
}


struct Mob;
struct Field {
    enum class T {
        PLAYER = '@',
        GOAL   = 'x',
        WALL   = '#',
        EMPTY  = ' ',
        ROAD   = '*',
        OPEN   = '?',
        MOB    = 'M',
    };
    using enum T;
    static auto to_string(T field) -> std::string
    {
        return std::string(1, static_cast<char>(field));
    }

    std::string face;
    T type = T::EMPTY;

    auto to_string() const -> std::string
    {
        if (face.empty()) {
            return to_string(type);
        }
        return face;
    }
    auto operator=(T const t) -> Field&
    {
        face = "";
        type = t;
        return *this;
    }
    auto operator=(std::pair<std::string, T> f) -> Field&
    {
        face = f.first;
        type = f.second;
        return *this;
    }
    auto operator==(T const t) const -> bool
    {
        return t == type;
    }
    /* explicit */ Field(T t) : face{""}, type{t}
    {}
};
using board_type = std::vector<std::vector<Field>>;
auto load_board(std::string input_file) -> board_type
{
    auto board = board_type{};
    {
        auto in   = std::ifstream{input_file};
        auto line = std::string{};
        std::getline(in, line);
        auto const row_size = std::stoull(line);
        while (std::getline(in, line)) {
            line.resize(row_size, ' ');
            board.push_back({});

            for (auto const each : line) {
                switch (each) {
                case '@':
                    board.back().push_back(Field::PLAYER);
                    break;
                case 'x':
                    board.back().push_back(Field::GOAL);
                    break;
                case '#':
                    board.back().push_back(Field::WALL);
                    break;
                default:
                    board.back().push_back(Field::EMPTY);
                    break;
                }
            }
        }
    }
    return board;
}
auto get_field(board_type& board, size_t x, size_t y) -> Field&
{
    return board.at(y).at(x);
}
auto get_field(board_type const& board, size_t x, size_t y) -> Field
{
    return board[y][x];
}
auto print_board(board_type const& board, size_t x, size_t y) -> void
{
    for (auto const& row : board) {
        set_cursor(x, y);
        for (auto const& f : row) {
            write(1, f.to_string());
        }
        ++y;
    }
}
auto is_valid_field(board_type const& board, std::pair<size_t, size_t> p)
    -> bool
{
    auto const [x, y] = p;
    return (y < board.size() and x < board.front().size());
}
auto is_valid_move(board_type const& board, std::pair<size_t, size_t> p) -> bool
{
    if (!is_valid_field(board, p)) {
        return false;
    }
    auto const [x, y] = p;
    auto const f      = get_field(board, x, y);
    if (f == Field::WALL or f == Field::MOB or f == Field::PLAYER) {
        return false;
    }
    return true;
}
struct Game_state {
    board_type board;
    struct {
        int x;
        int y;
    } monkey_pr;

    std::vector<std::unique_ptr<Mob>> mobs;

    auto detect_collision(int const x, int const y) -> bool
    {
        return is_valid_move(board, {x, y});
    }
};

auto neighbors(Game_state& game, size_t x, size_t y) -> std::vector<pos_type>
{
    auto close_friends = std::vector<std::pair<size_t, size_t>>{};
    auto up            = std::pair<size_t, size_t>{x, y + 1};
    auto down          = std::pair<size_t, size_t>{x, y - 1};
    auto left          = std::pair<size_t, size_t>{x - 1, y};
    auto right         = std::pair<size_t, size_t>{x + 1, y};
    for (auto each : {up, down, left, right}) {
        if (game.detect_collision(each.first, each.second)) {
            close_friends.push_back(each);
        }
    }
    return close_friends;
}
struct Mob {
    std::string face;
    int x{2};
    int y{2};

    Mob(std::string f) : face{f}
    {}
    Mob(std::string f, int x_p, int y_p) : face{f}, x{x_p}, y{y_p}
    {}
    virtual ~Mob()
    {}

    auto detect_collision(Game_state const& game_state) -> bool
    {
        return is_valid_move(game_state.board, {x, y});
    }

    auto put(Game_state& game) -> void
    {
        get_field(game.board, x, y) = {face, Field::MOB};
    }

    auto move(Game_state& game, size_t f_x, size_t f_y) -> void
    {
        get_field(game.board, x, y) = Field::EMPTY;
        x                           = f_x;
        y                           = f_y;
        get_field(game.board, x, y) = {face, Field::MOB};
    }

    auto erase(Game_state& game) const -> void
    {
        get_field(game.board, x, y) = Field::EMPTY;
    }

    virtual auto frame_action(Game_state&) -> void
    {}
};

struct Horizontal : Mob {
    using Mob::Mob;
    bool right = true;

    auto true_frame_action(Game_state& game) -> void
    {
        auto f_x = x;
        if (right) {
            ++f_x;
        } else {
            --f_x;
        }
        if (!is_valid_move(game.board, {f_x, y})) {
            if (right) {
                --f_x;
            } else {
                ++f_x;
            }
            right = not right;
        }
        move(game, f_x, y);
    }
    auto frame_action(Game_state& game) -> void override
    {
        true_frame_action(game);
        /* set_cursor(game.map_size.x + 2, 8); */
        /* write(1, "Hertical"); */
        /* set_cursor(game.map_size.x + 2, 9); */
        /* write(1, std::to_string(x) + ":" + std::to_string(y) + "  "); */
    }
};

struct Vertical : Mob {
    using Mob::Mob;
    bool up = true;
    auto true_frame_action(Game_state& game) -> void
    {
        if (up) {
            ++y;
        } else {
            --y;
        }
        if (detect_collision(game)) {
            if (up) {
                --y;
            } else {
                ++y;
            }
            up = not up;
        }
    }
    auto frame_action(Game_state& game) -> void override
    {
        true_frame_action(game);
        /* set_cursor(game.map_size.x + 2, 6); */
        /* write(1, "Vertical"); */
        /* set_cursor(game.map_size.x + 2, 7); */
        /* write(1, std::to_string(x) + ":" + std::to_string(y) + "  "); */
    }
};

struct God_Mob : Mob {
    using Mob::Mob;
    bool right             = true;
    int back_to_position   = 0;
    bool i_want_to_go_back = false;
    bool up_check          = false;
    auto reconstruct_path(std::map<pos_type, pos_type> came_from, pos_type goal)
        -> std::stack<pos_type>
    {
        auto total_path = std::stack<pos_type>{};
        /* goal = came_from[goal]; */
        while (came_from.contains(goal)) {
            goal = came_from[goal];
            total_path.push(goal);
        }
        total_path.pop();
        return total_path;
    }
    auto set_score(std::map<pos_type, float>& gscore,
                   pos_type pair,
                   float value) -> void
    {
        gscore.insert({pair, value});
    }

    auto get_score(std::map<pos_type, float>& gscore, pos_type pair) -> float
    {
        if (gscore.contains(pair)) {
            return gscore.at(pair);
        }
        return INFINITY;
    }
    auto a_star(Game_state& game) -> std::optional<std::stack<pos_type>>
    {
        auto mob = std::pair<size_t, size_t>(x, y);
        auto goal =
            std::pair<size_t, size_t>(game.monkey_pr.x, game.monkey_pr.y);
        auto path = std::optional<std::stack<pos_type>>{};

        auto const h = [goal](size_t x, size_t y) -> double {
            auto const a = std::llabs(goal.first - x);
            auto const b = std::llabs(goal.second - y);
            return std::sqrt((a * a) + (b * b));
        };

        auto open_queue =
            std::priority_queue<std::pair<double, pos_type>,
                                std::vector<std::pair<double, pos_type>>,
                                std::greater<std::pair<double, pos_type>>>{};
        auto open_set           = std::set<pos_type>{};
        auto const push_to_open = [&open_queue, &open_set](
                                      double prority,
                                      pos_type position) -> void {
            if (open_set.contains(position)) {
                return;
            }
            open_queue.push({prority, position});
            open_set.insert(position);
        };
        auto const pop_from_open = [&open_queue, &open_set]() -> pos_type {
            auto const node = open_queue.top().second;
            open_queue.pop();
            open_set.erase(node);
            return node;
        };

        auto l = neighbors(game, 0, 0);

        push_to_open(0, mob);

        auto came_from = std::map<pos_type, pos_type>{};

        std::map<pos_type, float> gscore;
        set_score(gscore, mob, 0);

        std::map<pos_type, float> fscore;
        set_score(fscore, mob, h(mob.first, mob.second));

        while (!open_set.empty()) {
            auto const current = pop_from_open();
            if (current == goal) {
                path = reconstruct_path(came_from, goal);
                break;
            }

            for (auto neighbor :
                 neighbors(game, current.first, current.second)) {
                auto tentative_gscore = get_score(gscore, current) + 1;
                if (tentative_gscore < get_score(gscore, neighbor)) {
                    came_from[neighbor] = current;
                    gscore[neighbor]    = tentative_gscore;
                    fscore[neighbor] =
                        tentative_gscore + h(neighbor.first, neighbor.second);
                    push_to_open(tentative_gscore, neighbor);
                }
            }
        }
        return path;
    }
    auto true_frame_action(Game_state& game) -> void
    {
        auto path = a_star(game);
        if (!path){
            //FIXME Why in first turn it crashed
            return;
        }
        if (path->empty()){
            return;
        }
        auto step = path->top();
        move(game, step.first, step.second);
    }
    auto frame_action(Game_state& game) -> void override
    {
        true_frame_action(game);
        /* set_cursor(game.map_size.x + 2, 10); */
        /* write(1, std::string{"God_mob"}); */
        /* set_cursor(game.map_size.x + 2, 11); */
        /* write(1, std::to_string(x) + ":" + std::to_string(y) + "  "); */
    }
};

enum class Direction {
    North,
    South,
    East,
    West,
};

struct Snake : Mob {
    using Mob::Mob;
    std::optional<Direction> direction;
    size_t steps = 5;
    auto frame_action(Game_state& game) -> void override
    {
        // FIXME rand(3) -> std::random
        auto d = direction.value_or(static_cast<Direction>(rand() % 4));
        auto const pr_x = x;
        auto const pr_y = y;
        switch (d) {
        case Direction::North:
            /* face = "\e[35mX\e[0m"; */
            y--;
            break;
        case Direction::South:
            /* face = "\e[32mX\e[0m"; */
            y++;
            break;
        case Direction::East:
            /* face = "\e[33mX\e[0m"; */
            x++;
            break;
        case Direction::West:
            /* face = "\e[36mX\e[0m"; */
            x--;
            break;
        }
        if (detect_collision(game)) {
            x     = pr_x;
            y     = pr_y;
            steps = 1;
        }

        direction = direction.value_or(d);

        if (--steps == 0) {
            steps = 5;
            while (true) {
                auto next = static_cast<Direction>(rand() % 4);
                if (next != direction) {
                    direction = next;
                    break;
                }
            }
        }
    }
};

auto main() -> int
{
    system("stty -icanon -echo");
    clear();
    hide_cursor();

    Game_state game_state{};
    game_state.board = load_board("plansza.txt");
    set_cursor(1, 1);
    create_map(game_state.board.front().size(), game_state.board.size());

    set_cursor(20,20);
    std::cout << game_state.board.front().size() << "x" << game_state.board.size()<<std::endl;

    auto& mobs = game_state.mobs;
    mobs.push_back(std::make_unique<Mob>("@", 3, 3));
    /* mobs.push_back(std::make_unique<Horizontal>("H", 4, 4)); */
    mobs.push_back(std::make_unique<God_Mob>("G", 10, 5));
    /* mobs.push_back(std::make_unique<Snake>("X", 11, 11)); */
    /* mobs.push_back(std::make_unique<Vertical>("V", 7, 12)); */
    auto& monkey = *mobs.front();
    game_state.monkey_pr.x = monkey.x;
    game_state.monkey_pr.y = monkey.y;
    /* monkey.put(game_state); */
    /* print_board(game_state.board,2,2); */

    for (auto &each : mobs){
        each->put(game_state);
    }
    print_board(game_state.board, 2, 2);

    constexpr auto EMPTY_INPUT = char{'\0'};
    auto buff                  = EMPTY_INPUT;

    do {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        auto const nfds = 0 + 1;

        timeval timeout{2, 200000};

        if (select(nfds, &readfds, nullptr, nullptr, &timeout) == -1) {
            break;
        }

        buff = EMPTY_INPUT;
        if (FD_ISSET(0, &readfds)) {
            read(0, &buff, 1);
        }
        auto f_x = monkey.x;
        auto f_y = monkey.y;
        switch (buff) {
        case 'w':
            f_y--;
            break;
        case 's':
            f_y++;
            break;
        case 'd':
            f_x++;
            break;
        case 'a':
            f_x--;
            break;
        case 'm':
            f_x = 2;
            f_y = 2;
            break;
        default:
            break;
        }
        for (auto& mob : mobs) {
            mob->frame_action(game_state);
        }

        if (!is_valid_move(game_state.board, {f_x, f_y})) {
            f_x = monkey.x;
            f_y = monkey.y;
        }
        game_state.monkey_pr.x = monkey.x;
        game_state.monkey_pr.y = monkey.y;

        monkey.move(game_state, f_x, f_y);
        print_board(game_state.board, 2, 2);
        /* std::this_thread::sleep_for(std::chrono::microseconds{timeout.tv_usec}); */
    } while (buff != 'q');
    system("reset");
    return 0;
}

/* auto old_main() -> int */
/* { */
/*     system("stty -icanon -echo"); */
/*     clear(); */
/*     hide_cursor(); */

/*     auto const MAP_WIDTH  = 35; */
/*     auto const MAP_HEIGHT = 20; */
/*     set_cursor(1, 1); */
/*     create_map(MAP_WIDTH, MAP_HEIGHT); */
/*     /1* create_map(8,8,4,6); *1/ */

/*     Game_state game_state{}; */
/*     game_state.board = load_board("plansza.txt"); */

/*     auto& mobs = game_state.mobs; */
/*     mobs.push_back(std::make_unique<Mob>("@")); */
/*     mobs.push_back(std::make_unique<Horizontal>("H", 4, 4)); */
/*     mobs.push_back(std::make_unique<God_Mob>("G", 4, 4)); */
/*     mobs.push_back(std::make_unique<Snake>("X", 11, 11)); */
/*     mobs.push_back(std::make_unique<Vertical>("V", 7, 12)); */
/*     auto& monkey = *mobs.front(); */
/*     monkey.display(); */

/*     constexpr auto EMPTY_INPUT = char{'\0'}; */
/*     auto buff                  = EMPTY_INPUT; */
/*     do { */
/*         fd_set readfds; */
/*         FD_ZERO(&readfds); */
/*         FD_SET(0, &readfds); */
/*         auto const nfds = 0 + 1; */

/*         timeval timeout{0, 200000}; */

/*         if (select(nfds, &readfds, nullptr, nullptr, &timeout) == -1) { */
/*             break; */
/*         } */

/*         buff = EMPTY_INPUT; */
/*         if (FD_ISSET(0, &readfds)) { */
/*             read(0, &buff, 1); */
/*         } */

/*         for (auto& mob : mobs) { */
/*             mob->erase(); */
/*         } */
/*         game_state.monkey_pr.x = monkey.x; */
/*         game_state.monkey_pr.y = monkey.y; */
/* auto const pr_x        = monkey.x; */
/* auto const pr_y        = monkey.y; */
/*         switch (buff) { */
/*         case 'w': */
/*             monkey.y--; */
/*             break; */
/*         case 's': */
/*             monkey.y++; */
/*             break; */
/*         case 'd': */
/*             monkey.x++; */
/*             break; */
/*         case 'a': */
/*             monkey.x--; */
/*             break; */
/*         case 'm': */
/*             monkey.x = 2; */
/*             monkey.y = 2; */
/*             break; */
/*         default: */
/*             break; */
/*         } */

/*         if (monkey.detect_collision(game_state)) { */
/*             monkey.x = pr_x; */
/*             monkey.y = pr_y; */
/*         } */

/* for (auto& mob : mobs) { */
/*     mob->frame_action(game_state); */
/* } */

/*         for (auto& mob : mobs) { */
/*             mob->restrain(MAP_WIDTH, MAP_HEIGHT); */
/*             mob->display(); */
/*         } */
/*         monkey.display();  // Monkey always on top */

/*         set_cursor(MAP_WIDTH + 2, 1); */
/*         write(1, */
/*               std::to_string(monkey.x) + ":" + std::to_string(monkey.y) + "
 * "); */
/*     } while (buff != 'q'); */

/*     system("reset"); */
/*     return 0; */
/* } */
