#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class EventfdList{
    public:
        constexpr static int MAX_CALLBACKS = 65500;
        Epoll & epoll;
        int epoll_eventfd;
        std::mutex mtx;
        int head;
        int tail;
        std::atomic<int> count;
        std::unique_lock<std::mutex> locker;
        std::array<std::function<void()>, MAX_CALLBACKS> callbacks;

        std::queue<std::function<void()>> queues;


    public:
        EventfdList(Epoll & epoll)
            : epoll{epoll}
            , epoll_eventfd{-1}
            , head{0}
            , tail{0}
            , count{0}
            , locker{mtx, std::defer_lock} { }

        int 
        InitEventFd(){
            if(epoll_eventfd != -1) {
                return epoll_eventfd;
            }
            epoll_eventfd = eventfd(0, EFD_NONBLOCK);
            if(epoll_eventfd == -1)
                return -1;

            using std::placeholders::_1;
            auto event = Event{};
            event.fd = epoll_eventfd;
            event.pop = std::bind(&EventfdList::Pop, this, _1);

            struct epoll_event ev;
            ev.data.fd = epoll_eventfd;
            ev.events = EPOLLIN ;

            epoll.AddEvent(event, ev);

            return epoll_eventfd;
        }
        void
        Pop(const struct epoll_event & ev){
            uint64_t res;
            int ret = read(ev.data.fd, &res, sizeof(uint64_t));
            // printf("pop read res:%lu\n", res);
            if(ret == -1){
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                return;
            }

            // printf("res:%lu, head:%d, count:%d\n", res, head, tail);

            for(uint64_t i = 0 ; i < res ; i++){
                callbacks[head]();
                callbacks[head] = nullptr;
                head++;
                if(head == MAX_CALLBACKS) head = 0;
                count--;
            }
        }
        void 
        SendEvent(std::function<void()> && callback){
            if(InitEventFd() == -1) return;
            
            
            locker.lock();
            if(count == MAX_CALLBACKS) { 
                locker.unlock();
                return;
            }

            count++;
            callbacks[tail++] = callback;
            if(tail == MAX_CALLBACKS){
                tail = 0;
            }
            locker.unlock();

            uint64_t v = 1;
            Error error;
            ssize_t ret = write(epoll_eventfd, &v, sizeof(uint64_t));
            if(ret == -1){
                error = Error{strerror(errno)};
            }
        }

        int 
        InitEventFd2(){
            if(epoll_eventfd != -1) {
                return epoll_eventfd;
            }
            epoll_eventfd = eventfd(0, EFD_NONBLOCK);
            if(epoll_eventfd == -1)
                return -1;

            using std::placeholders::_1;
            auto event = Event{};
            event.fd = epoll_eventfd;
            event.pop = std::bind(&EventfdList::Pop2, this, _1);

            struct epoll_event ev;
            ev.data.fd = epoll_eventfd;
            ev.events = EPOLLIN ;

            epoll.AddEvent(event, ev);

            return epoll_eventfd;
        }
        void
        Pop2(const struct epoll_event & ev){
            uint64_t res;
            int ret = read(ev.data.fd, &res, sizeof(uint64_t));

            if(ret == -1){
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                return;
            }


            for(uint64_t i = 0 ; i < res ; i++){
                auto & func = queues.front();
                func();
                func = nullptr;
                mtx.lock();
                queues.pop();
                mtx.unlock();
            }
        }
        void 
        SendEvent2(std::function<void()> && callback){
            if(InitEventFd2() == -1) return;
            
            mtx.lock();
            queues.push(std::move(callback));
            mtx.unlock();

            uint64_t v = 1;
            Error error;
            ssize_t ret = write(epoll_eventfd, &v, sizeof(uint64_t));
            if(ret == -1){
                error = Error{strerror(errno)};
            }
        }


    };
}