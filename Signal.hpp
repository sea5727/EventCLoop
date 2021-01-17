#pragma once

#include "EventCLoop.hpp"
#include <sys/signalfd.h>

namespace EventCLoop
{
    template<int N>
    class Signal{
        Epoll & epoll;
        int signal_fd;
        std::array<int, N> signals;
    public:
        Signal(Epoll & epoll, std::array<int, N> signals)
            : epoll{epoll}
            , signal_fd{-1}
            , signals{signals} {

            std::cout << "signals size : " << N << std::endl;

            sigset_t mask;

            sigemptyset(&mask);

            for(auto & signal : signals){
                sigaddset(&mask, signal);
            }

            int ret = sigprocmask(SIG_SETMASK, &mask, NULL);
            if(ret == -1)
                throw std::runtime_error(std::string{"sigprocmask error "} + std::string{strerror(errno)});
            
            signal_fd = ::signalfd(-1, &mask, 0);
            if(signal_fd == -1)
                throw std::runtime_error(std::string{"signalfd error "} + std::string{strerror(errno)});

        }
        ~Signal(){
            if(signal_fd != -1){
                close(signal_fd);
            }
        }

        void
        AsyncSignal(std::function<void(int /*signalno*/)> callback){
            using std::placeholders::_1;
            auto event = Event{};
            event.fd = signal_fd;
            event.pop = std::bind(&Signal::AsyncSignalPop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = signal_fd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }
    private:
        void
        AsyncSignalPop(const struct epoll_event & ev, std::function<void(int /*signalno*/)> callback){
            struct signalfd_siginfo fdsi;
            int res = read(ev.data.fd , &fdsi, sizeof(fdsi));

            if(res != sizeof(fdsi)){
                throw std::runtime_error(std::string{"signal system error:"} + std::string{strerror(errno)});
            }

            auto no = fdsi.ssi_signo;
            std::cout << "signal : " << no << std::endl;

            callback(no);
        }

    };
}