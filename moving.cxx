#include <unistd.h>
#include <stdlib.h>
#include <string>

auto clear() ->void
{
    write(1,"\033[2J",4);
}

auto set_cursor(int x ,int y) ->void
{
    auto set ="\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
    write(1,set.c_str(),set.size());
}

auto icon() ->void
{
    auto monkey="@\n";
    write(1,&monkey,2);
}

auto main() ->int
{
    system("stty -icanon -echo");

    int x=1 ,y =1;
    clear();
    set_cursor(x,y);
    icon();

    while(true){
        char buff;
        read(0,&buff,1);
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
                x=1;
                y=1;
                break;
            default:
                break;
        }
        set_cursor(x,y);
        icon();

    }

    system("stty icanon echo");
    return 0;
}
