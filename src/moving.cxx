#include <stdlib.h>
#include <unistd.h>

#include <string>

auto clear() -> void
{
    write(1, "\033[2J", 4);
}

auto set_cursor(int x, int y) -> void
{
    auto set = "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1, set.c_str(), set.size());
}

auto icon(char c = '@') -> void
{
    auto monkey = c + std::string{"\n"};
    write(1, monkey.c_str(), monkey.size());
}
auto hide_cursor() -> void
{
    auto set = std::string{"\e[?25l"};
    write(1, set.c_str(), set.size());
}
auto main() -> int
{
    system("stty -icanon -echo");

    int x = 1, y = 1;
    clear();
    set_cursor(x, y);
    icon();
    hide_cursor();

    char buff;
    do {
        read(0, &buff, 1);
        set_cursor(x, y);
        icon(' ');
        switch (buff) {
        case 'w':
            y--;
            break;
        case 's':
            y++;
            break;
        case 'd':
            x++;
            break;
        case 'a':
            x--;
            break;
        case 'm':
            x = 1;
            y = 1;
            break;
        default:
            break;
        }
        set_cursor(x, y);
        icon();

    } while (buff != 'q');

    system("stty icanon echo");
    return 0;
}
