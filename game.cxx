#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/select.h>

#include "map.h"

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

auto set_cursor(int x ,int y) ->void
{
    auto set ="\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1,set.c_str(),set.size());
}

auto icon(char c ='@') ->void
{
    auto monkey=c + std::string{"\n"};
    write(1,monkey.c_str(),monkey.size());
}
auto hide_cursor() ->void
{
    auto set =std::string{"\e[?25l"};
    write(1,set.c_str(),set.size());
}

auto main() ->int
{
    auto const MAP_WIDTH=25;
    auto const MAP_HEIGHT=15;
    system("stty -icanon -echo");

    int x=2 ,y =2;
    clear();
    set_cursor(1,1);
    create_map(MAP_WIDTH,MAP_HEIGHT);
    set_cursor(x,y);
    icon();
    hide_cursor();

    char buff;
    do{
        read(0,&buff,1);
        set_cursor(x,y);
        icon(' ');
        switch(buff){
            case 'w':
                y-- ;
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
                x=2;
                y=2;
                break;
            default:
                break;
        }

        x = std::max(1+1,x);
        x = std::min(MAP_WIDTH-1,x);

        y = std::max(1+1,y);
        y = std::min(MAP_HEIGHT-1,y);

        set_cursor(x,y);
        icon();

        set_cursor(MAP_WIDTH+2,1);
        write(1,std::to_string(x)+":"+std::to_string(y)+"  ");
    }while(buff !='q');

    system("stty icanon echo");
    return 0;
}
