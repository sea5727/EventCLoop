#include "EventCLoop/EventCLoop.hpp"

int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    auto epoll = EventCLoop::Epoll{};

    int i = 0;
    while(1){
        std::cout << "i : " << i << std::endl;
        epoll.Run();
        i++;
    }
    

}