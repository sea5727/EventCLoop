#pragma once

#include "EventCLoop.hpp"
#include <sys/signalfd.h>

namespace EventCLoop
{
    class Signal{
        Epoll & epoll;
        Event event;
        int signal_fd;
    public:
        Signal(Epoll & epoll)
            : epoll{epoll}
            , event{}
            , signal_fd{-1} {

            sigset_t mask;

            sigemptyset(&mask);

            for(int i = SIGHUP; i <= SIGTSTP ; ++i){
                sigaddset(&mask, i);
            }

            int ret = sigprocmask(SIG_SETMASK, &mask, NULL);
            if(ret == -1)
                throw std::runtime_error(std::string{"sigprocmask error "} + std::string{strerror(errno)});
            
            signal_fd = ::signalfd(-1, &mask, 0);
            if(signal_fd == -1)
                throw std::runtime_error(std::string{"signalfd error "} + std::string{strerror(errno)});

        }

        void
        AsyncSignal(std::function<void()> callback){
            using std::placeholders::_1;
            event.fd = signal_fd;
            event.pop = std::bind(&Signal::AsyncSignalPop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = signal_fd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }

        void
        AsyncSignalPop(const struct epoll_event & ev, std::function<void()> callback){
            struct signalfd_siginfo fdsi;
            int res = read(ev.data.fd , &fdsi, sizeof(fdsi));

            if(res != sizeof(fdsi)){
                throw std::runtime_error(std::string{"signal system error:"} + std::string{strerror(errno)});
            }

            auto no_sig = fdsi.ssi_signo;
            std::cout << "signal : " << no_sig << std::endl;

            CreateCore();

            if(no_sig == SIGINT){

            }
            else if(no_sig == SIGQUIT){
            }
            else {
            }
            exit(0);
        }

        void
        CreateCore(){
            char s_log_name[512] = { 0, };
            auto pid = getpid();

            snprintf(s_log_name, sizeof(s_log_name), "./down_TMF_%d.core", pid);
            printf("[CORE] Core file path[%s] \n", s_log_name);

            FILE *s_log = NULL;

            s_log = fopen(s_log_name, "w");

            if (!s_log) {
                printf(" s_log == NULL -> return\n");
                return;
            }

            printf(" s_log != NULL\n");
            char cmd[128] = { 0, };
            snprintf(cmd, sizeof(cmd), "/usr/bin/pstack %d", pid);

            char str[512] = { 0, };
            FILE *ptr = popen(cmd, "r");
            printf("popen\n");
            if (ptr != NULL) {
                while (1) {
                    memset(str, 0x00, sizeof(str));
                    if (fgets(str, 512, ptr) == NULL) break;
                    if (s_log) fprintf(s_log, "[PSTACK] %s", str);
                }
                pclose(ptr);
            }
            if (s_log) fclose(s_log);
            printf("end\n");


        }
    };
}