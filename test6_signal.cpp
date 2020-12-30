#include "EventCLoop/EventCLoop.hpp"
#include <thread>
#include <execinfo.h>
#include <sys/wait.h>



int main(int argc, char * argv[]){

    
    auto epoll = EventCLoop::Epoll{};
    auto signal = EventCLoop::Signal<3>{epoll, {SIGHUP,SIGUSR1, SIGUSR2}};

    signal.AsyncSignal([](int signalno){
        switch(signalno){
            case SIGHUP:
                break;
            case SIGUSR1:
                break;
            case SIGUSR2:
                break;
            default:
                break;
        }
    });

    while(1){
        epoll.Run();
    }
    

}