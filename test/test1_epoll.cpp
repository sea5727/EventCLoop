#include "EventCLoop.hpp"

int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    auto epoll = EventCLoop::Epoll{};

    while(1){
        epoll.Run();
    }
    

}