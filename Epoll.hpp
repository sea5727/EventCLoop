#pragma once

#include "Event.hpp"

namespace EventCLoop{
    class Epoll {
        constexpr static unsigned int EpollSize = 1024;
        constexpr static unsigned int EpollTimeout = 1024;
        int epollfd;
        std::map<int, Event & > events;
        struct epoll_event ev[EpollSize];
    public:
        Epoll(){
            epollfd = epoll_create1(0);
            if(epollfd == -1){
                throw std::logic_error("epoll_create1 fail");
            }
        }

        void
        AddEvent(Event & event, struct epoll_event ev){
            std::cout << "[" << now_str() << "] ========== start Call AddEvent : " << event.fd << std::endl;
            if(epoll_ctl(epollfd, EPOLL_CTL_ADD, event.fd, &ev) == -1){
                throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
            }
            events.insert({event.fd, event});
        }
        void
        DelEvent(int eventfd){
            std::cout << "[" << now_str() << "] ========== Call DelEvent : " << eventfd << std::endl;
            events.erase(eventfd);
            if(epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, nullptr) == -1){
                std::cout << "epoll_ctl EPOLL_CTL_ADD fail " << strerror(errno) << std::endl;
                // throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
            }
        }
        void
        ModEvent(int eventfd, struct epoll_event ev){
            std::cout << "[" << now_str() << "] ========== Call ModEvent : " << eventfd << std::endl;

            if(epoll_ctl(epollfd, EPOLL_CTL_MOD, eventfd, &ev) == -1){
                throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
                std::cout << "epoll_ctl EPOLL_CTL_MOD fail " << strerror(errno) << std::endl;
            }
        }

        void
        Run(){
            auto count = epoll_wait(epollfd, ev, EpollSize, EpollTimeout);
            for(int i = 0 ; i < count ; ++i){
                std::cout << "[" << now_str() << "] fd:" << ev[i].data.fd << " Epoll event.. event: " << ev[i].events << std::endl;
                int key = ev[i].data.fd;
                auto value = events.find(key);
                if(value == events.end()){
                    continue;
                }
                if(value->second.pop != nullptr)
                    value->second.pop(ev[i]);
                
            }
        }


    };



}
