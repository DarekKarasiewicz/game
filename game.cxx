#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/select.h>

auto main() -> int
{

    char buff[100];
    auto sz = ssize_t{};
    memset(buff,0,100);

    while(true){
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(0,&readfds);
        auto const nfds= 0 + 1;

        timeval timeout{5,0};

        if(select(nfds,&readfds,nullptr,nullptr,&timeout) == -1){
            break;
        }
        if(FD_ISSET(0,&readfds)){
            sz= read(0,buff,100);
        }

        write(1,buff,sz);
    }
    return 0;
}
