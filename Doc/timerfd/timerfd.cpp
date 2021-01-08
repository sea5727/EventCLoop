#include <iostream>
#include <thread>
#include <chrono>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

constexpr static int EPOLL_SIZE = 1024;
constexpr static int EPOLL_TIMEOUT = 1000;

int
mono_timer_one(int epollfd){
    auto mono = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec ts;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = 1;
    ts.it_value.tv_nsec = 500 * 1000;

    if(timerfd_settime(mono, 0, &ts, NULL) < 0){
        throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
    }

    struct epoll_event mono_ev;
    mono_ev.data.fd = mono;
    mono_ev.events = EPOLLIN;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, mono, &mono_ev) < 0){
        throw std::runtime_error("epoll_ctl fail " + std::string{strerror(errno)});
    }
    return mono;
}

int
mono_timer_interval(int epollfd){
    auto mono = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec ts;
    ts.it_interval.tv_sec = 1;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = 1; // value 가 모두 0이면 동작하지 않는듯.
    ts.it_value.tv_nsec = 0;

    if(timerfd_settime(mono, 0, &ts, NULL) < 0){
        throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
    }

    struct epoll_event mono_ev;
    mono_ev.data.fd = mono;
    mono_ev.events = EPOLLIN;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, mono, &mono_ev) < 0){
        throw std::runtime_error("epoll_ctl fail " + std::string{strerror(errno)});
    }
    return mono;
}


int main(int argc, char * argv[]){
    
    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }

    int timer_one = mono_timer_one(epollfd);
    int timer_interval = mono_timer_interval(epollfd);


    struct epoll_event ev[EPOLL_SIZE];

    while(1){
        auto count = epoll_wait(epollfd, ev, EPOLL_SIZE, EPOLL_TIMEOUT);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
            if(fd == timer_one){
                uint64_t res;
                int ret = read(ev[i].data.fd, &res, sizeof(uint64_t));
                std::cout << "[TIMER] mono timer one call!! ret : " << ret << std::endl;
            }
            else if(fd == timer_interval){
                uint64_t res;
                int ret = read(ev[i].data.fd, &res, sizeof(uint64_t));
                std::cout << "[TIMER] mono timer interval call!! ret : " << ret << std::endl;
            }
        }
    }
    return 0;
}