#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <queue>
#include <utility>
#include <functional>

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
auto print_board(board_type const& board) ->void
{
    for(auto const& row : board ){
        for(auto const f : row){
                std::cout << to_string(f);
        }
        std::cout << "|\n";
    }
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

    using pos_type = std::pair<size_t,size_t>;
    pos_type player, goal;
    {
        for (size_t y = 0; y < board.size(); ++y) {
            for (size_t x = 0; x < board.front().size(); ++x) {
                switch (board[y][x]) {
                case Field::PLAYER:
                    player.first = x;
                    player.second = y;
                    break;
                case Field::GOAL:
                    goal.first = x;
                    goal.second = y;
                    break;
                default:
                    break;
                }
            }
        }
        std::cout << to_string(get_field(board, player.first, player.second)) << "\n";
        std::cout << to_string(get_field(board, goal.first, goal.second)) << "\n";
    }
    print_board(board);

    auto const h = [goal](size_t x, size_t y) -> double
    {
        auto const a = std::llabs(goal.first - x);
        auto const b = std::llabs(goal.second - y);
        return std::sqrt((a*a) + (b*b));
    };
    static_cast<void>(h);

    auto q = std::priority_queue<
        std::pair<double, pos_type>,
        std::vector<std::pair<double, pos_type>>,
        std::greater<std::pair<double, pos_type>>>{};
    q.push({1.0,player});
    q.push({0.3,player});

    std::cout << q.top().first <<"\n";

    return 0;
}
