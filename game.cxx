#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <vector>
#include <memory>

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

auto hide_cursor() ->void
{
    auto set =std::string{"\e[?25l"};
    write(1,set.c_str(),set.size());
}

struct Mob{
    std::string face;
    int x{2};
    int y{2};

    Mob(std::string f): face{f}
    {
    }

    auto restrain(int max_x,int max_y) ->void
    {
        x = std::max(1+1,x);
        x = std::min(max_x-1,x);

        y = std::max(1+1,y);
        y = std::min(max_y-1,y);
    }

    auto display() const ->void
    {
        set_cursor(x,y);
        write(1,face+'\n');
    }

    auto erase() const ->void
    {
        set_cursor(x,y);
        write(1," \n");
    }
};
auto main() ->int
{
    system("stty -icanon -echo");
    clear();
    hide_cursor();

    auto const MAP_WIDTH=25;
    auto const MAP_HEIGHT=15;
    set_cursor(1,1);
    create_map(MAP_WIDTH,MAP_HEIGHT);

    auto mobs =std::vector<std::unique_ptr<Mob>>{};
    mobs.push_back(std::make_unique<Mob>("@"));
    auto& monkey = *mobs.front();
    monkey.display();

    char buff;
    do{
        read(0,&buff,1);
        monkey.erase();
        switch(buff){
            case 'w':
                monkey.y-- ;
                break;
            case 's':
                monkey.y++;
                break;
            case 'd':
                monkey.x++;
                break;
            case 'a':
                monkey.x--;
                break;
            case 'm':
                monkey.x=2;
                monkey.y=2;
                break;
            default:
                break;
        }
        monkey.restrain(MAP_WIDTH,MAP_HEIGHT);
        monkey.display();

        set_cursor(MAP_WIDTH+2,1);
        write(1,std::to_string(monkey.x)+":"+std::to_string(monkey.y)+"  ");
    }while(buff !='q');

    system("stty icanon echo");
    return 0;
}
