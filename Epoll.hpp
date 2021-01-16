#pragma once

#include "Event.hpp"

namespace EventCLoop{
    
    class Epoll {
        constexpr static unsigned int EpollSize = 1024;
        constexpr static unsigned int EpollTimeout = 1024;
        int epollfd;
        std::map<int, Event & > events;
        std::array<Event, 65000> allevent;
        struct epoll_event ev[EpollSize];
    public:
        Epoll()  {
            Error error;
            Init(error);
            if(error)
                throw error;

        }
        Epoll(Error & error){
            Init(error);
        }

        inline
        void
        AddEvent(Event & event, struct epoll_event & ev){
            Error error;
            AddEvent(event, ev, error);
            if(error)
                throw error;
        }

        inline
        void
        AddEvent(Event & event, struct epoll_event & ev, Error & error) noexcept {
            std::cout << "[" << now_str() << "] ========== start Call AddEvent : " << event.fd << std::endl;
            if(epoll_ctl(epollfd, EPOLL_CTL_ADD, event.fd, &ev) == -1){
                error = Error{std::string{"epoll_ctl EPOLL_CTL_ADD fail : "} + strerror(errno)};
                return;
            }
            events.insert({event.fd, event});
            allevent[event.fd] = event;
        }

        inline
        void
        DelEvent(int eventfd){
            Error error;
            DelEvent(eventfd, error);
            if(error) throw error;
        }

        inline
        void
        DelEvent(int eventfd, Error & error) noexcept {
            std::cout << "[" << now_str() << "] ========== Call DelEvent : " << eventfd << std::endl;
            events.erase(eventfd);
            allevent[eventfd].clear();
            if(epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, nullptr) == -1){
                error = Error{std::string{"epoll_ctl EPOLL_CTL_ADD fail : "} + strerror(errno)};
            }
        }

        inline
        void
        ModEvent(int eventfd, struct epoll_event & ev){
            Error error;
            ModEvent(eventfd, ev, error);
            if(error) throw error;
        }

        inline
        void
        ModEvent(int eventfd, struct epoll_event & ev, Error & error) noexcept {
            std::cout << "[" << now_str() << "] ========== Call ModEvent : " << eventfd << std::endl;
            if(epoll_ctl(epollfd, EPOLL_CTL_MOD, eventfd, &ev) == -1){
                Error{std::string{"epoll_ctl EPOLL_CTL_MOD fail : "} + strerror(errno)};
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
    private:
        void
        Init(Error & error){
            epollfd = epoll_create1(0);
            if(epollfd == -1){                
                error = Error{std::string{"epoll_create1 : "} + strerror(errno)};
            }
        }


    };



}
