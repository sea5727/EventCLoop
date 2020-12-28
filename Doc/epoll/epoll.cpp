#include <iostream>

#include <sys/epoll.h>

constexpr static int EPOLL_SIZE = 1024;
constexpr static int EPOLL_TIMEOUT = 1000;

int main(int argc, char * argv[]){
    
    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }

    struct epoll_event ev[EPOLL_SIZE];

    while(1){
        auto count = epoll_wait(epollfd, ev, EPOLL_SIZE, EPOLL_TIMEOUT);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
            // TODO
        }
    }
    return 0;
}