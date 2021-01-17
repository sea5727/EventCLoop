#include <iostream>
#include <thread>
#include <chrono>


#include "EventCLoop.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


class MyApp{
    EventCLoop::Epoll & epoll;
    EventCLoop::TcpConnector connector;
    std::string ip;
    uint16_t port;
public:
    MyApp(
        EventCLoop::Epoll & epoll, 
        const std::string & ip, 
        const uint16_t port)
        : epoll{epoll}
        , connector{epoll}
        , ip{ip}
        , port{port} { }
        
    void
    run(){
        std::cout << "run!! try async_connect" << std::endl;
        connector.async_connect(ip, port, [this](EventCLoop::Error & error, int fd){
            std::cout << "async_connect " << error.what() << std::endl; 
            if(error){
               std::cout << "async_connect error !!" << error.what() << std::endl; 
               auto timer = std::make_shared<EventCLoop::Timer>(epoll);
               timer->initOneTimer(1, 0);
               std::cout << "timer start" << error.what() << std::endl; 
               timer->async_wait([timer, this](EventCLoop::Error & error){
                   std::cout << "timer callabck???" << error.what() << std::endl; 
                   if(error){
                       //timer fail
                   }
                    run();
               });
               return;
            }
            std::cout << "async_connect success.. fd : " << fd << std::endl;

            using std::placeholders::_1;
            using std::placeholders::_2;
            using std::placeholders::_3;
            
            auto session = std::make_shared<EventCLoop::TcpSession>(epoll, fd);
            session->async_read([session, this](int fd, char * buffer, size_t len){
                std::cout << "async_read len : " << len << std::endl;
                if(len == 0){
                    session->clear_session();
                }
                read_handler(fd, buffer, len);
            });

        });
    }
    void
    read_handler(int fd, char * buffer, ssize_t len){
        if(len == 0){
            auto timer = std::make_shared<EventCLoop::Timer>(epoll);
            timer->initOneTimer(1, 0);
            timer->async_wait([timer, this](EventCLoop::Error & error){
                if(error){
                    //timer fail
                }
                run();
            });
            return;
        }

        std::cout << "buffer : " << buffer << std::endl;

    }
};
int main(int argc, char * argv[]){
    auto epoll = EventCLoop::Epoll{};
    auto myapp = MyApp{epoll, "192.168.0.35", 12345};
    // auto myapp = MyApp{epoll, "223.130.195.95", 12345};
    myapp.run();

    while(1){
        std::cout << "main worker run" << std::endl;
        epoll.Run();
    }
    return 0;
}