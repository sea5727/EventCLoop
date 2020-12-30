#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <execinfo.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char * argv[]){
    int event_fd = eventfd(0, EFD_NONBLOCK);
    if(event_fd == -1)
        throw std::runtime_error(std::string{"eventfd create fail "} + std::string{strerror(errno)});

    std::cout << "[Eventfd] create eventfd : " << event_fd << std::endl;


    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }

    struct epoll_event eventfd_ev;
    eventfd_ev.data.fd = event_fd;
    eventfd_ev.events = EPOLLIN;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, event_fd, &eventfd_ev);
    
    struct epoll_event ev[1024];


    int loop = 0;
    while(1){
        auto count = epoll_wait(epollfd, ev, 1024, 1000);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
                if(fd == event_fd){
                uint64_t res;
                int ret = read(event_fd, &res, sizeof(uint64_t));
                std::cout << "SendEventPop : " << ret << ", res : " << res << std::endl;
            }
        }
        if(loop == 3){
            uint64_t count = 1;
            ssize_t ret = write(event_fd, &count, sizeof(uint64_t));
            if(ret == -1) 
                throw std::logic_error("write fail");
        }

        loop++;

    }
}