#include <iostream>
#include <thread>
#include <chrono>
#include "EventCLoop/EventCLoop.hpp"

class MyServer{
public:
    std::map<int, std::shared_ptr<EventCLoop::TcpSession>> sessions;
    EventCLoop::Epoll & epoll;
    std::string ip;
    uint16_t port;
    EventCLoop::Acceptor acceptor;

    MyServer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip) 
        : epoll{epoll} 
        , ip{ip}
        , port{port}
        , acceptor{epoll, port, ip} { }

    void
    run(){
        do_accept();
    }
    void
    do_accept(){
        acceptor.async_accept([&](int sessionfd, std::string ip, uint16_t port){
            handle_accept(sessionfd, ip, port);
        });
    }
    void
    handle_accept(int sessionfd, std::string ip, uint16_t port){
        std::cout << "[APP] Tcp Listen Success remote:" << ip << ":" << port << std::endl;
        auto session = std::make_shared<EventCLoop::TcpSession>(epoll, sessionfd);
        sessions.insert(std::make_pair(sessionfd, session));
        session->async_read([session, this](int fd, char * buffer, size_t readlen){
            if(readlen == 0){
                return;   
            }
            char * p = nullptr;
            auto mylen = session->buffer.dispatch_chunk(p, nullptr);
            // auto mylen = session->buffer.dispatch_chunk(p, 
            //     [](char * buffer, ssize_t len){
            //         if(len < 8) return 0;
            //         //proc
            //         return 10;
            //     });
            if(mylen == 0){
                std::cout << "[APP] Need More..\n";
                return;
            }
            std::cout << "[APP] Async_read callback!! len : " << mylen << std::endl;
            do_read();
        });
    }

    void
    do_read(){
        
    }


};


int main(int argc, char * argv[]){

    auto epoll = EventCLoop::Epoll{};

    auto myserver = MyServer{epoll, 12345, "192.168.0.35"};
    myserver.run();

    while(1){
        epoll.Run();

    }
    return 0;
}