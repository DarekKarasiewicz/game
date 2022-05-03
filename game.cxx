#include <optional>
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

struct Mob;

struct Terrain
{
    int x;
    int y;
    size_t height;
    size_t width;

    Terrain(int x_p, int y_p,size_t h, size_t w)
        : x{x_p}
        , y{y_p}
        , height{h}
        , width{w}
        {}
};

struct Game_state{
    struct {
        int x;
        int y;
    } map_size;

    std::vector<Terrain> terrain;

    std::vector<std::unique_ptr<Mob>> mobs;
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

    auto detect_collision(Game_state const& game_state) ->bool
    {
        for (auto const& terrain : game_state.terrain){
            if(x<terrain.x){
                continue;
            }
            if(x>terrain.x + static_cast<int>(terrain.height)){
                continue;
            }
            if(y<terrain.y){
                continue;
            }
            if(y>terrain.y + static_cast<int>(terrain.width)){
                continue;
            }
            return true;
        }
        return false;
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

    virtual auto frame_action(Game_state&) ->void
    {}
};

struct Horizontal :Mob
{
    using Mob::Mob;
    bool right=true;
    auto frame_action(Game_state &game) ->void override
    {
        if(x==game.map_size.x-1 ){
            right=false;
        }
        else if( x==2){
            right =true;
        }
        if(right){
            ++x;
        }
        else{
            --x;
        }
        if(detect_collision(game)){
            if(right){
                --x;
            }
            else{
                ++x;
            }
            right=not right;
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
        if(y==game.map_size.y-1 ){
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

enum class Direction
{
    North,
    South,
    East,
    West,
};

struct Snake :Mob
{
    using Mob::Mob;
    std::optional<Direction> direction;
    size_t steps=5;
    auto frame_action(Game_state &game) ->void override
    {
        //FIXME rand(3) -> std::random
        auto d=direction.value_or(static_cast<Direction>(rand() % 4));

        switch (d){
            case Direction::North:
                face="\e[35mX\e[0m";
                y--;
                break;
            case Direction::South:
                face="\e[32mX\e[0m";
                y++;
                break;
            case Direction::East:
                face="\e[33mX\e[0m";
                x++;
                break;
            case Direction::West:
                face="\e[36mX\e[0m";
                x--;
                break;
        }

        direction=direction.value_or(d);

        if (--steps == 0){
            steps=5;
            while (true){
                auto next =static_cast<Direction>(rand() % 4);
                if (next != direction){
                    direction=next;
                    break;
                }
            }
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
    /* create_map(8,8,4,6); */

    Game_state game_state{};
    game_state.map_size.x=MAP_WIDTH;
    game_state.map_size.y=MAP_HEIGHT;
    game_state.terrain.emplace_back(8,3,4,6);
    game_state.terrain.emplace_back(20,10,2,2);

    for (auto& terrain : game_state.terrain){
        create_map(terrain.x,terrain.y,terrain.width,terrain.height);
    }

    auto& mobs =game_state.mobs;
    mobs.push_back(std::make_unique<Mob>("@"));
    mobs.push_back(std::make_unique<Horizontal>("H",4,4));
    mobs.push_back(std::make_unique<Snake>("X",11,11));
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
