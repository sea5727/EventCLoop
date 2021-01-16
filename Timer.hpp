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

            Error error;
            Init(error);
            if(error) throw error;
        }

        Timer(Epoll & epoll, Error & error)
            : epoll{epoll}
            , event{}
            , timerfd{-1} {
            
            Init(error);

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
            Error error;
            initOneTimer(sec, nsec, error);
            if(error) throw error;
        }

        void
        initOneTimer(unsigned int sec, unsigned int nsec, Error & error) noexcept {
            if(nsec > 1'000'000'000){
                sec += nsec % 1'000'000'000;
                nsec /= 1'000'000'000;
            }
            
            struct itimerspec ts;
            ts.it_interval.tv_sec = 0;
            ts.it_interval.tv_nsec = 0;
            ts.it_value.tv_sec = sec;
            ts.it_value.tv_nsec = nsec;

            if(timerfd_settime(timerfd, 0, &ts, NULL) < 0){
                error = Error{std::string{"timerfd_settime fail : "} + strerror(errno)};
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

            Error error;
            epoll.AddEvent(event, ev, error);
            if(error) {
                auto event_fd = Eventfd{epoll};
                event_fd.SendEvent([callback, error]{
                    auto errro_ = error;
                    callback(errro_);
                });
            }
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
                if(ret < 0)
                    error = Error{strerror(errno)};
            }
            
            callback(error);

            epoll.DelEvent(ev.data.fd);
            close(ev.data.fd);
            event.clear();
        }
        
        void
        Init(Error & error) {
            timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
            if(timerfd == -1)
                error = Error{std::string{"timerfd_create fail "} + strerror(errno)};
        }



    };
}