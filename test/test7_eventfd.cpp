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
            auto start = std::chrono::steady_clock::now();
            for(int i = 0; i < 50000 ; ++i){
                int ret = event_fd.SendEvent([i]{ // main thread 에서 실행된다.
                    // std::cout << "Send Event !! i :" << i << std::endl;
                });
            }
            auto end = std::chrono::steady_clock::now();
            auto diff = end - start;
            std::cout << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

            while(1){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }


        }

    }); 


    int count = 0;
    while(1){
        // std::cout << "[Id:" << std::this_thread::get_id() <<  "] main sleep_for 1" << std::endl;
        epoll.Run();
        count += 1;
    }
}