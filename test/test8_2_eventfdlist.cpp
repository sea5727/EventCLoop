#include "EventCLoop.hpp"
#include <thread>
#include <execinfo.h>
#include <sys/wait.h>


int main(int argc, char * argv[]){

    auto threadpool = std::vector<std::thread>{};

    auto epoll = EventCLoop::Epoll{};
    auto eventfdlist = EventCLoop::EventfdList{epoll};

    threadpool.emplace_back([&eventfdlist]{
        while(1){
            try{
                auto start = std::chrono::steady_clock::now();
                for(int i = 0 ; i < 100000; i ++){
                    eventfdlist.SendEvent2([i, &start]{
                        if(i == 100000 - 1){
                            auto end = std::chrono::steady_clock::now();
                            auto diff = end - start;
                            std::cout << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
                        }

                    });
                }

            }
            catch(EventCLoop::Error & error){
                std::cout << "threadpool error : " << error.what() << std::endl;
            }
            while(1){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

    }); 


    int count = 0;
    while(1){
        try{
            // std::cout << "[Id:" << std::this_thread::get_id() <<  "] main sleep_for 1" << std::endl;
            epoll.Run();
            count += 1;
        }
        catch(EventCLoop::Error & err){
            std::cout << "runm error : " << err.what() << std::endl;
        }

    }
}