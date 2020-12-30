#include "EventCLoop/EventCLoop.hpp"
#include <thread>
#include <execinfo.h>
#include <sys/wait.h>



int main(int argc, char * argv[]){

    
    auto epoll = EventCLoop::Epoll{};
    auto signal = EventCLoop::Signal<5>{epoll, {1,3,4,5}};

    auto signals = std::array{SIGHUP,SIGUSR1, SIGUSR2};

    while(1){
        std::cout << "main thread.." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    

}