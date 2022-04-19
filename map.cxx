#include <unistd.h>
#include <string>

namespace {
    auto write(int fd,std::string const s) -> ssize_t
    {
        return ::write(fd,s.c_str(),s.size());
    }
}

auto clear() ->void
{
    write(1,"\033[2J",4);
}

auto set_cursor(int x,int y) ->void
{
    auto set ="\033["+ std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1,set);
}

auto create_map(size_t width, size_t height) ->void
{
    auto border = std::string(width,'#')+'\n';
    auto wall ='#' + std::string(width-2,' ') + '#' + '\n';
    write(1,border);
    for(size_t i=0;i<(height-2);i++){
        write(1,wall);
    }
    write(1,border);


}


auto main() ->int
{
    system("stty -icanon -echo");

    clear();
    set_cursor(1,1);

    create_map(100,22);



    system("stty icanon echo");

    return 0;
}
