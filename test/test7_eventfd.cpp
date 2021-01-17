#include "EventCLoop.hpp"
#include <thread>
#include <execinfo.h>
#include <sys/wait.h>


int main(int argc, char * argv[]){

    auto threadpool = std::vector<std::thread>{};

    auto epoll = EventCLoop::Epoll{};
    auto event_fd = EventCLoop::Eventfd{epoll};

    threadpool.emplace_back([&event_fd]{
        while(1){
            
            std::cout << "[Id:" << std::this_thread::get_id() <<  "] sleep_for 1" << std::endl;
            event_fd.SendEvent([]{ // main thread 에서 실행된다.
                std::cout << "Send Event !! 1" << std::endl;
            });
            event_fd.SendEvent([]{ // main thread 에서 실행된다.
                std::cerr << "Send Event !! 2"  << std::endl;
            });
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    }); 


    int count = 0;
    while(1){
        std::cout << "[Id:" << std::this_thread::get_id() <<  "] main sleep_for 1" << std::endl;
        epoll.Run();
        count += 1;
    }
}