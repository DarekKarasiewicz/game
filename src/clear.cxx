#include <unistd.h>


auto main() -> int
{
    auto clear = "\033[2J";
    write(1, clear, 4);
    auto set_cursor = "\033[1;1H";
    write(1, set_cursor, 6);
    return 0;
}
