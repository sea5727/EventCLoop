#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class Eventfd{
    public:
        Epoll & epoll;
    public:
        Eventfd(Epoll & epoll)
            : epoll{epoll} { }

        int
        SendEvent(std::function<void()> && callback){
            
            auto event_fd = eventfd(0, EFD_NONBLOCK);
            if(event_fd == -1){
                return -1;
                // error = Error{std::string{"eventfd create fail :"} + strerror(errno)};
            }


            using std::placeholders::_1;
            auto event = Event{};
            event.fd = event_fd;
            event.pop = std::bind(&Eventfd::SendEventPop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = event_fd;
            ev.events = EPOLLIN ;

            epoll.AddEvent(event, ev);

            uint64_t count = 1;
            Error error;
            ssize_t ret = write(event_fd, &count, sizeof(uint64_t));
            if(ret == -1){
                error = Error{strerror(errno)};
                return -1;
            }

            return 0;
        }

        void
        SendEventPop(const struct epoll_event & ev, std::function<void()> & callback){
            
            uint64_t res;
            int ret = read(ev.data.fd, &res, sizeof(uint64_t));
            if(ret == -1){
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                return;
            }

            for(uint64_t i = 0; i < res ; ++i){
                callback();
            }


        }

    };
}