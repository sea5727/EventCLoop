#include "EventCLoop/EventCLoop.hpp"

int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    auto epoll = EventCLoop::Epoll{};

    auto sockets = std::vector<int>{};

    for(int i = 0 ; i < 5 ; ++i){
        auto fd = socket(AF_INET, SOCK_STREAM, 0);

        auto event = std::make_shared<EventCLoop::Event>();
        event->fd = fd; 
        event->pop = nullptr;

        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN;

        epoll.AddEvent(event, ev);
    }

    while(1){
        epoll.Run();
    }
    

}