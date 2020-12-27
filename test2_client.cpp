#include <iostream>
#include <thread>
#include <chrono>
#include "EventCLoop/EventCLoop.hpp"

class MyApp{
public:
    EventCLoop::Epoll & epoll;
    EventCLoop::TcpConnect connector;
    MyApp(EventCLoop::Epoll & epoll, std::string ip, uint16_t port)
        : epoll{epoll}
        , connector{epoll, ip, port} { }

    void
    run(){
        connector.async_connect([this](EventCLoop::Error error){
            handle_connect(error);
        });
    }
    void
    handle_connect(EventCLoop::Error error){
        if(error){
            std::cout << "async connect error..." << error.what() << std::endl;
            auto timer = std::make_shared<EventCLoop::Timer>(epoll);
            timer->initOneTimer(1, 0);
            timer->async_wait([=]{
                timer;
                run();
            });
            
            return;
        }
        connector.async_read([this](int fd, char * buffer, size_t len){
            handle_read(fd, buffer, len);
        });

    }
    void
    handle_read(int fd, char * buffer, size_t len){
        if(len == 0){
            std::cout << "async_read callback... len = 0" << std::endl;
            
            run();
            return;
        }
        std::cout << "async_read callback... len : " << len << std::endl;
    }
};

int main(int argc, char * argv[]){
    std::cout << "Hello test1\n";

    auto epoll = EventCLoop::Epoll{};
    auto myapp = MyApp{epoll, "192.168.0.36", 12345};
    myapp.run();

    while(1){
        epoll.Run();
        // std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    return 0;
}