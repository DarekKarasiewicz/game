#include <unistd.h>

#include <sstream>
#include <string>

namespace {
auto write(int fd, std::string const s) -> ssize_t
{
    return ::write(fd, s.c_str(), s.size());
}
auto set_cursor(int x, int y) -> void
{
    auto set = "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1, set);
}
}  // namespace

auto multiply_string(size_t count, std::string s) -> std::string
{
    auto out = std::ostringstream{};
    for (size_t i = 0; i < count; ++i) {
        out << s;
    }
    return out.str();
}

auto create_map(size_t width, size_t height) -> void
{
    auto border = multiply_string(width - 2, "─");
    auto top    = "┌" + border + "┐" + '\n';
    auto bot    = "└" + border + "┘" + '\n';
    auto wall   = "│" + std::string(width - 2, ' ') + "│" + '\n';
    write(1, top);
    for (size_t i = 0; i < (height - 2); i++) {
        write(1, wall);
    }
    write(1, bot);
}

auto create_map(int x, int y, size_t width, size_t height) -> void
{
    auto border = multiply_string(width - 2, "─");
    auto top    = "┌" + border + "┐" + '\n';
    auto bot    = "└" + border + "┘" + '\n';
    auto wall   = "│" + std::string(width - 2, ' ') + "│" + '\n';

    set_cursor(x, y);
    write(1, top);
    for (size_t i = 0; i < (height - 2); ++i) {
        set_cursor(x, ++y);
        write(1, wall);
    }
    set_cursor(x, ++y);
    write(1, bot);
}
