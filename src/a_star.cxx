#include <fstream>
#include <iostream>
#include <string>
#include <vector>

enum class Field {
    PLAYER = '@',
    GOAL   = 'x',
    WALL   = '#',
    EMPTY  = ' ',
};

auto main() -> int
{
    auto board = std::vector<std::vector<Field>>{};
    {
        auto in   = std::ifstream{"plansza.txt"};
        auto line = std::string{};
        std::getline(in, line);
        auto const row_size = std::stoull(line);
        while (std::getline(in, line)) {
            line.resize(row_size, ' ');
            std::cout << line << "|\n";

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
    struct {
        size_t x, y;
    } player, goal;
    for (size_t y = 0; y < board.size(); ++y) {
        for (size_t x = 0; x < board.front().size(); ++x) {
            switch (board[y][x]) {
            case Field::PLAYER:
                player.x = x;
                player.y = y;
                break;
            case Field::GOAL:
                goal.x = x;
                goal.y = y;
                break;
            default:
                break;
            }
        }
    }
    std::cout << player.x << " | " << player.y << "\n";
    std::cout << goal.x << " | " << goal.y << "\n";
    return 0;
}
