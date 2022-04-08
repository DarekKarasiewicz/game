#include <stdlib.h>
#include <unistd.h>
#include <string>


auto main() -> int
{
    while(true){
        char buff[100];
        auto const sz= read(0,buff,100);
        write(1,buff,sz);
    }
    return 0;
}
