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

struct Game_state{
    /* using Mob::Mob; */
    int map_size_x;
    int map_size_y;

    int current_p_x;
    int current_p_y;
    /* std::vector<std::unique_ptr<Mob>> mobs; */
};

struct Mob{
    std::string face;
    int x{2};
    int y{2};

    Mob(std::string f): face{f}
    {}
    Mob(std::string f,int x_p, int y_p)
        : face{f}
        ,x{x_p}
        ,y{y_p}
    {}
    virtual ~Mob()
    {}

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

    virtual auto frame_action(int a) ->void
    {}

    virtual auto frame_action(Game_state&) ->void
    {}
};

struct Horizontal :Mob
{
    using Mob::Mob;
    bool right=true;
    bool left=false;
    auto frame_action(Game_state &game) ->void override
    {
        if(x==game.map_size_x-1 ){
            left =true;
            right=false;
        }
        else if( x==2){
            right =true;
            left=false;
        }
        if(right){
            x++;
        }
        if(left){
            x--;
        }
    }
};

struct Vertical :Mob
{
    using Mob::Mob;
    bool up=true;
    bool left=false;
    auto frame_action(Game_state &game) ->void override
    {
        if(y==game.map_size_y-1 ){
            left =true;
            up=false;
        }
        else if( y==2){
            up =true;
            left=false;
        }
        if(up){
            y++;
        }
        if(left){
            y--;
        }
    }
};

struct Snake :Mob
{
    using Mob::Mob;
    bool up=true;
    bool down=true;
    bool right=true;
    bool left=false;

    int i=5;
    auto frame_action(Game_state &game) ->void override
    {
        int random_direction= rand() % 4;
        if(i==5){
        switch(random_direction){
            case 0:
                i=0;
                left=true;
                up=false;
                down=false;
                right=false;
                break;
            case 1:
                i=0;
                left=false;
                up=true;
                down=false;
                right=false;
                break;
            case 2:
                i=0;
                left=false;
                up=false;
                down=true;
                right=false;
                break;
            case 3:
                i=0;
                left=false;
                up=false;
                down=false;
                right=true;
                break;
            }
        }
        if(left){
            x--;
            i++;
        }
        if(right){
            x++;
            i++;
        }
        if(up){
            y--;
            i++;
        }
        if(down){
            y++;
            i++;
        }
        if(y==game.map_size_y-1 or y==2 or x==2 or x==game.map_size_x-1){
            i=5;
            frame_action(game);
        }
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
    Game_state game_state{};
    game_state.map_size_x=MAP_WIDTH;
    game_state.map_size_y=MAP_HEIGHT;

    auto mobs =std::vector<std::unique_ptr<Mob>>{};
    mobs.push_back(std::make_unique<Mob>("@"));
    mobs.push_back(std::make_unique<Horizontal>("H",4,4));
    mobs.push_back(std::make_unique<Snake>("X",11,11));
    game_state.current_p_x=4;
    game_state.current_p_y=4;
    mobs.push_back(std::make_unique<Vertical>("V",6,12));
    auto& monkey = *mobs.front();
    monkey.display();

    char buff;
    do{
        read(0,&buff,1);

        for(auto& mob :mobs){
            mob->erase();
        }

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

        for(auto& mob :mobs){
            mob->frame_action(game_state);
        }

        for(auto& mob :mobs){
            mob->restrain(MAP_WIDTH,MAP_HEIGHT);
            mob->display();
        }
        monkey.display();  //Monkey always on top

        set_cursor(MAP_WIDTH+2,1);
        write(1,std::to_string(monkey.x)+":"+std::to_string(monkey.y)+"  ");
    }while(buff !='q');

    system("stty icanon echo");
    return 0;
}
