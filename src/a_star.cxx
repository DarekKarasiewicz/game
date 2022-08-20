#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <utility>
#include <vector>

enum class Field {
    PLAYER = '@',
    GOAL   = 'x',
    WALL   = '#',
    EMPTY  = ' ',
    ROAD   = '*',
    OPEN   = '?',
};
auto to_string(Field field) -> std::string
{
    return std::string(1, static_cast<char>(field));
}

using board_type = std::vector<std::vector<Field>>;
auto get_field(board_type& board, size_t x, size_t y) -> Field&
{
    return board[y][x];
}
auto get_field(board_type const& board, size_t x, size_t y) -> Field
{
    return board[y][x];
}
auto print_board(board_type const& board) -> void
{
    for (auto const& row : board) {
        for (auto const f : row) {
            std::cout << to_string(f);
        }
        std::cout << "|\n";
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
    if (f == Field::WALL) {
        return false;
    }
    return true;
}
auto neighbors(board_type const& board, size_t x, size_t y)
    -> std::vector<std::pair<size_t, size_t>>
{
    auto close_friends = std::vector<std::pair<size_t, size_t>>{};
    auto up            = std::pair<size_t, size_t>{x, y + 1};
    auto down          = std::pair<size_t, size_t>{x, y - 1};
    auto left          = std::pair<size_t, size_t>{x - 1, y};
    auto right         = std::pair<size_t, size_t>{x + 1, y};
    for (auto each : {up, down, left, right}) {
        if (is_valid_move(board, each)) {
            close_friends.push_back(each);
        }
    }
    return close_friends;
}

auto set_score(std::map<std::pair<size_t, size_t>, float>& gscore,
               std::pair<size_t, size_t> pair,
               float value) -> void
{
    gscore.insert({pair, value});
}

auto get_score(std::map<std::pair<size_t, size_t>, float>& gscore,
               std::pair<size_t, size_t> pair) -> float
{
    if (gscore.contains(pair)) {
        return gscore.at(pair);
    }
    return INFINITY;
}
using pos_type = std::pair<size_t, size_t>;
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

auto main() -> int
{
    auto board = board_type{};
    {
        auto in   = std::ifstream{"plansza.txt"};
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

    pos_type player, goal;
    {
        for (size_t y = 0; y < board.size(); ++y) {
            for (size_t x = 0; x < board.front().size(); ++x) {
                switch (board[y][x]) {
                case Field::PLAYER:
                    player.first  = x;
                    player.second = y;
                    break;
                case Field::GOAL:
                    goal.first  = x;
                    goal.second = y;
                    break;
                default:
                    break;
                }
            }
        }
        std::cout << to_string(get_field(board, player.first, player.second))
                  << "\n";
        std::cout << to_string(get_field(board, goal.first, goal.second))
                  << "\n";
    }
    print_board(board);

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
                                  double prority, pos_type position) -> void {
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
    auto l = neighbors(board, 0, 0);
    std::cout << l.size() << "\n";

    push_to_open(0, player);

    auto came_from = std::map<pos_type, pos_type>{};

    std::map<pos_type, float> gscore;
    set_score(gscore, player, 0);

    std::map<pos_type, float> fscore;
    set_score(fscore, player, h(player.first, player.second));

    auto path = std::optional<std::stack<pos_type>>{};
    while (!open_set.empty()) {
        auto const current = pop_from_open();
        auto& f            = get_field(board, current.first, current.second);
        if (f != Field::PLAYER and f != Field::GOAL) {
            f = Field::EMPTY;
        }
        if (current == goal) {
            path = reconstruct_path(came_from, goal);
            break;
        }

        for (auto neighbor : neighbors(board, current.first, current.second)) {
            auto tentative_gscore = get_score(gscore, current) + 1;
            if (tentative_gscore < get_score(gscore, neighbor)) {
                came_from[neighbor] = current;
                gscore[neighbor]    = tentative_gscore;
                fscore[neighbor] =
                    tentative_gscore + h(neighbor.first, neighbor.second);
                push_to_open(tentative_gscore, neighbor);
                auto& f = get_field(board, neighbor.first, neighbor.second);
                if (f != Field::GOAL) {
                    f = Field::OPEN;
                }
            }
        }

        print_board(board);
        std::cout << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds{167});
    }

    if (!path.has_value()) {
        return 1;
    }
    while (!path->empty()) {
        auto step = path->top();
        path->pop();

        get_field(board, step.first, step.second) = Field::ROAD;
    }
    print_board(board);

    return 0;
}
