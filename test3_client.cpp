#include <iostream>
#include <thread>
#include <chrono>
#include "EventCLoop/EventCLoop.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class MyApp{
public:
    EventCLoop::Epoll & epoll;
    EventCLoop::TcpConnect connector;
    MyApp(EventCLoop::Epoll & epoll, std::string ip, uint16_t port)
        : epoll{epoll}
        , connector{epoll, port, ip} { }

    void
    run(){
        connector.async_connect([this](EventCLoop::Error error){
            std::cout << "this : " << (void *)this << ", async_connect handler start" << std::endl;
            if(error){
               std::cout << "async_connect error !!" << error.what() << std::endl; 
               return;
            }

            connector.async_read(std::bind(&MyApp::read_handler, this, _1, _2, _3));
            std::cout << "this : " << (void *)this << ", async_connect handler end" << std::endl;
        });
    }
    void
    read_handler(int fd, char * buffer, size_t len){
        if(len == 0){
            std::cout << "this : " << (void *)this << ", read_handler len == 0 clear ! " << std::endl;
            connector.clear_session();
            return;
        }

        std::cout << "this : " << (void *)this << ", read_handler len: " << len << std::endl;
    }
};

int main(int argc, char * argv[]){
    auto epoll = EventCLoop::Epoll{};
    auto myapp = MyApp{epoll, "192.168.0.35", 12345};
    myapp.run();

    while(1){
        epoll.Run();
    }
    return 0;
}