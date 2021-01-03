#pragma once



#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"

#include <sched.h>

namespace EventCLoop
{
    class Timer{
        Epoll & epoll;
        Event event;
        int timerfd;
    public:
        Timer(Epoll & epoll)
            : epoll{epoll}
            , event{}
            , timerfd{-1} {
            timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
            if(timerfd == -1)
                throw std::runtime_error("timerfd_create fail " + std::string{strerror(errno)});
            std::cout << "[TIMER] fd : " << timerfd << std::endl;

            // struct sched_param schedparm;

            // memset(&schedparm, 0, sizeof(schedparm));
            // schedparm.sched_priority = 1; // lowest rt priority
            // sched_setscheduler(0, SCHED_FIFO, &schedparm);
        }
        ~Timer(){
            std::cout << "[TIMER] Delete Timer...fd:" << event.fd << std::endl;
            if(!event.isCleared()){
                std::cout << "[TIMER] not Clear.. -> DelEvent, clear, close.. :" << event.fd << std::endl;
                epoll.DelEvent(event.fd);
                close(event.fd);
                event.clear();
                
            }
            
        }

        void
        initOneTimer(unsigned int sec, unsigned int nsec){
            
            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));
            ts.it_interval.tv_sec = 0;
            ts.it_interval.tv_nsec = 0;
            ts.it_value.tv_sec = sec;
            ts.it_value.tv_nsec = nsec;

            if(timerfd_settime(timerfd, 0, &ts, NULL) < 0){
                throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
            }
        }
        [[deprecated("don't use")]]
        void
        initIntervalTimer(
            unsigned int interval_sec,
            unsigned int interval_nsec,
            unsigned int sec, 
            unsigned int nsec){

            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));
            ts.it_interval.tv_sec = interval_sec;
            ts.it_interval.tv_nsec = interval_nsec;
            ts.it_value.tv_sec = sec;
            ts.it_value.tv_nsec = nsec;

            if(timerfd_settime(timerfd, 0, &ts, NULL) < 0){
                throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
            }
        }
        void
        async_wait(std::function<void(Error & )> callback){
            using std::placeholders::_1;
            event.fd = timerfd;
            event.pop = std::bind(&Timer::epoll_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = timerfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }

    private:
        void
        epoll_pop(const struct epoll_event & ev, std::function<void(Error & )> callback){
            std::cout << "[TIMER] poll pop!! fd:" << ev.data.fd << ", ev : " << ev.events << std::endl;

            Error error;
            if(ev.events & EPOLLERR){
                error = Error{strerror(errno)};
            }
            else {
                uint64_t res;
                int ret = read(ev.data.fd, &res, sizeof(uint64_t));
            }
            
            callback(error);

            epoll.DelEvent(ev.data.fd);
            close(ev.data.fd);
            event.clear();
            

            
            
        }



    };
}