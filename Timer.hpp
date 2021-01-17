#pragma once



#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"

#include <sched.h>

namespace EventCLoop
{
    class Timer{
        Epoll & epoll;
        int timerfd = -1;
    public:
        Timer() = default;
        Timer(Epoll & epoll)
            : epoll{epoll} {
            Error error;
            Init(error);
            if(error) throw error;
        }

        Timer(Epoll & epoll, Error & error) noexcept
            : epoll{epoll} {
            Init(error);
        }

        ~Timer(){
            if(timerfd != -1){
                epoll.DelEvent(timerfd);
                close(timerfd);
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
            auto event = Event{};
            event.fd = timerfd;
            event.pop = std::bind(&Timer::epoll_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = timerfd;
            ev.events = EPOLLIN;

            Error error;
            epoll.AddEvent(event, ev, error);
            if(error) {
                auto event_fd = std::make_shared<Eventfd>(epoll);
                event_fd->SendEvent([callback, error, event_fd]{
                    auto errro_ = error;
                    callback(errro_);
                });
            }
        }

    private:
        void
        epoll_pop(const struct epoll_event & ev, std::function<void(Error & )> callback){

            Error error;
            if(ev.data.fd != timerfd){
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                return;
            }

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

            epoll.DelEvent(timerfd);
            close(timerfd);
            timerfd = -1;
        }
        
        void
        Init(Error & error) {
            timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
            if(timerfd == -1)
                error = Error{std::string{"timerfd_create fail "} + strerror(errno)};
        }



    };
}